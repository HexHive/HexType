//===-- HexTypePass.cpp -----------------------------------------------===//
//
// This file is a part of HexType, a type confusion detector.
//
// The HexTypePass has below two functions:
//   - Track Stack object allocation
//   - Track Global object allocation
// This pass will run after all optimization passes run
// The rest is handled by the run-time library.
//===------------------------------------------------------------------===//

#include "llvm/Analysis/CallGraph.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/HexTypeUtil.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/Instrumentation.h"

using namespace llvm;
#define MAXLEN 10000

namespace {

  struct HexType : public ModulePass {
    static char ID;
    HexType() : ModulePass(ID) {}

    HexTypeLLVMUtil *HexTypeUtilSet;
    CallGraph *CG;

    std::list<AllocaInst *> AllAllocaSet;
    std::map<AllocaInst *, IntrinsicInst *> LifeTimeEndSet;
    std::map<AllocaInst *, IntrinsicInst *> LifeTimeStartSet;
    std::map<Function *, std::vector<Instruction *> *> ReturnInstSet;
    std::map<Instruction *, Function *> AllAllocaWithFnSet;
    std::map<Function*, bool> mayCastMap;

    void getAnalysisUsage(AnalysisUsage &Info) const {
      Info.addRequired<CallGraphWrapperPass>();
    }

    Function *setGlobalObjUpdateFn(Module &M) {
      FunctionType *VoidFTy =
        FunctionType::get(Type::getVoidTy(M.getContext()), false);
      Function *FGlobal = Function::Create(VoidFTy,
                                           GlobalValue::InternalLinkage,
                                           "__init_global_object", &M);
      FGlobal->setUnnamedAddr(true);
      FGlobal->setLinkage(GlobalValue::InternalLinkage);
      FGlobal->addFnAttr(Attribute::NoInline);

      return FGlobal;
    }

    void handleFnPrameter(Module &M, Function *F) {
      if (F->empty() || F->getEntryBlock().empty() ||
          F->getName().startswith("__init_global_object"))
        return;

      Type *MemcpyParams[] = { HexTypeUtilSet->Int8PtrTy,
        HexTypeUtilSet->Int8PtrTy,
        HexTypeUtilSet->Int64Ty };
      Function *MemcpyFunc =
        Intrinsic::getDeclaration(&M, Intrinsic::memcpy, MemcpyParams);
      for (auto &a : F->args()) {
        Argument *Arg = dyn_cast<Argument>(&a);
        if (!Arg->hasByValAttr())
          return;
        Type *ArgPointedTy = Arg->getType()->getPointerElementType();
        if (HexTypeUtilSet->isInterestingType(ArgPointedTy)) {
          unsigned long size =
            HexTypeUtilSet->DL.getTypeStoreSize(ArgPointedTy);
          IRBuilder<> B(&*(F->getEntryBlock().getFirstInsertionPt()));
          Value *NewAlloca = B.CreateAlloca(ArgPointedTy);
          Arg->replaceAllUsesWith(NewAlloca);
          Value *Src = B.CreatePointerCast(Arg,
                                           HexTypeUtilSet->Int8PtrTy);
          Value *Dst = B.CreatePointerCast(NewAlloca,
                                           HexTypeUtilSet->Int8PtrTy);
          Value *Param[5] = { Dst, Src,
            ConstantInt::get(HexTypeUtilSet->Int64Ty, size),
            ConstantInt::get(HexTypeUtilSet->Int32Ty, 1),
            ConstantInt::get(HexTypeUtilSet->Int1Ty, 0) };
          B.CreateCall(MemcpyFunc, Param);
        }
      }
    }

    void collectLifeTimeInfo(Instruction *I) {
      if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(I))
        if ((II->getIntrinsicID() == Intrinsic::lifetime_start)
           || (II->getIntrinsicID() == Intrinsic::lifetime_end)) {
          ConstantInt *Size =
            dyn_cast<ConstantInt>(II->getArgOperand(0));
          if (Size->isMinusOne()) return;

          if (AllocaInst *AI =
             HexTypeUtilSet->findAllocaForValue(II->getArgOperand(1))) {
            if (II->getIntrinsicID() == Intrinsic::lifetime_start)
              LifeTimeStartSet.insert(std::pair<AllocaInst *,
                                   IntrinsicInst *>(AI, II));

            else if (II->getIntrinsicID() == Intrinsic::lifetime_end)
              LifeTimeEndSet.insert(std::pair<AllocaInst *,
                                 IntrinsicInst *>(AI, II));
          }
        }
    }

    void collectAllocaInstInfo(Instruction *I) {
      if (AllocaInst *AI = dyn_cast<AllocaInst>(I))
        if (HexTypeUtilSet->isInterestingType(AI->getAllocatedType())) {
          if (ClSafeStackOpt && HexTypeUtilSet->isSafeStackAlloca(AI))
            return;
          AllAllocaWithFnSet.insert(
            std::pair<Instruction *, Function *>(
              dyn_cast<Instruction>(I), AI->getParent()->getParent()));
          AllAllocaSet.push_back(AI);
        }
    }

    void handleAllocaAdd(Module &M) {
      for (AllocaInst *AI : AllAllocaSet) {
        Instruction *next = HexTypeUtilSet->findNextInstruction(AI);
        IRBuilder<> Builder(next);

        Value *ArraySizeF = NULL;
        if (ConstantInt *constantSize =
            dyn_cast<ConstantInt>(AI->getArraySize()))
          ArraySizeF =
            ConstantInt::get(HexTypeUtilSet->Int64Ty,
                             constantSize->getZExtValue());
        else {
          Value *ArraySize = AI->getArraySize();
          if (ArraySize->getType() != HexTypeUtilSet->Int64Ty)
            ArraySizeF = Builder.CreateIntCast(ArraySize,
                                               HexTypeUtilSet->Int64Ty,
                                               false);
          else
            ArraySizeF = ArraySize;
        }

        Type *AllocaType = AI->getAllocatedType();
        StructElementInfoTy offsets;
        HexTypeUtilSet->getArrayOffsets(AllocaType, offsets, 0);
        if(offsets.size() == 0) continue;

        std::map<AllocaInst *, IntrinsicInst *>::iterator LifeTimeStart, LifeTimeEnd;
        LifeTimeStart = LifeTimeStartSet.begin();
        LifeTimeEnd = LifeTimeStartSet.end();
        bool UseLifeTimeInfo = false;

        for (; LifeTimeStart != LifeTimeEnd; LifeTimeStart++)
          if (LifeTimeStart->first == AI) {
            IRBuilder<> BuilderAI(LifeTimeStart->second);
            HexTypeUtilSet->insertUpdate(&M, BuilderAI, "__update_stack_oinfo",
                                         AI, offsets,
                                         HexTypeUtilSet->DL.getTypeAllocSize(
                                           AllocaType),
                                         ArraySizeF, NULL, NULL);
            UseLifeTimeInfo = true;
          }

        if (UseLifeTimeInfo)
          continue;

        HexTypeUtilSet->insertUpdate(&M, Builder, "__update_stack_oinfo",
                                     AI, offsets,
                                     HexTypeUtilSet->DL.getTypeAllocSize(
                                       AllocaType),
                                     ArraySizeF, NULL, NULL);
      }
    }

    void findReturnInsts(Function *f) {
      std::vector<Instruction*> *TempInstSet = new std::vector<Instruction *>;
      for (inst_iterator j = inst_begin(f), E = inst_end(f); j != E; ++j)
        if (isa<ReturnInst>(&*j))
          TempInstSet->push_back(&*j);

      ReturnInstSet.insert(std::pair<Function *,
                           std::vector<Instruction *>*>(f, TempInstSet));
    }

    void handleAllocaDelete(Module &M) {
      std::map<Instruction *, Function *>::iterator LocalBegin, LocalEnd;
      LocalBegin = AllAllocaWithFnSet.begin();
      LocalEnd = AllAllocaWithFnSet.end();

      for (; LocalBegin != LocalEnd; LocalBegin++){
        Instruction *TargetInst = LocalBegin->first;
        AllocaInst *TargetAlloca = dyn_cast<AllocaInst>(TargetInst);

        Function *TargetFn = LocalBegin->second;

        std::vector<Instruction *> *FnReturnSet;
        FnReturnSet = ReturnInstSet.find(TargetFn)->second;

        std::vector<Instruction *>::iterator ReturnInstCur, ReturnInstEnd;
        ReturnInstCur = FnReturnSet->begin();
        ReturnInstEnd = FnReturnSet->end();
        DominatorTree dt = DominatorTree(*TargetFn);

        bool returnAI = false;
        for (; ReturnInstCur != ReturnInstEnd; ReturnInstCur++)
          if (dt.dominates(TargetInst, *ReturnInstCur)) {
            ReturnInst *returnValue = dyn_cast<ReturnInst>(*ReturnInstCur);
            if (returnValue->getNumOperands() &&
                returnValue->getOperand(0) == TargetAlloca) {
              returnAI = true;
              break;
            }
          }

        if (returnAI)
          continue;

        std::map<AllocaInst *, IntrinsicInst *>::iterator LifeTimeStart,
          LifeTimeEnd;
        LifeTimeStart = LifeTimeEndSet.begin();
        LifeTimeEnd = LifeTimeEndSet.end();
        bool lifeTimeEndEnable = false;
        for (; LifeTimeStart != LifeTimeEnd; LifeTimeStart++)
          if (LifeTimeStart->first == TargetAlloca) {
            IRBuilder<> BuilderAI(LifeTimeStart->second);
            HexTypeUtilSet->emitRemoveInst(&M, BuilderAI, TargetAlloca);
            lifeTimeEndEnable = true;
          }

        if (lifeTimeEndEnable)
          continue;

        ReturnInstCur = FnReturnSet->begin();
        ReturnInstEnd = FnReturnSet->end();

        for (; ReturnInstCur != ReturnInstEnd; ReturnInstCur++)
          if (dt.dominates(TargetInst, *ReturnInstCur)) {
            IRBuilder<> BuilderAI(*ReturnInstCur);
            HexTypeUtilSet->emitRemoveInst(&M, BuilderAI, TargetAlloca);
          }
      }
    }

    // This is typesan's optimization method
    // to reduce stack object tracing overhead
    bool mayCast(Function *F, std::set<Function*> &visited, bool *isComplete) {
      // Externals may cast
      if (F->isDeclaration())
        return true;

      // Check previously processed
      auto mayCastIterator = mayCastMap.find(F);
      if (mayCastIterator != mayCastMap.end())
        return mayCastIterator->second;

      visited.insert(F);

      bool isCurrentComplete = true;
      for (auto &I : *(*CG)[F]) {
        return true;
        Function *calleeFunction = I.second->getFunction();
        // Default to true to avoid accidental bugs on API changes
        bool result = false;
        // Indirect call
        if (!calleeFunction) {
          result = true;
          // Recursion detected, do not process callee
        } else if (visited.count(calleeFunction)) {
          isCurrentComplete = false;
          // Explicit call to checker
        } else if (
          calleeFunction->getName().find("__dynamic_casting_verification") !=
          StringRef::npos ||
          calleeFunction->getName().find("__type_casting_verification_changing") !=
          StringRef::npos ||
          calleeFunction->getName().find("__type_casting_verification") !=
          StringRef::npos) {
          result = true;
          // Check recursively
        } else {
          bool isCalleeComplete;
          result = mayCast(calleeFunction, visited, &isCalleeComplete);
          // Forbid from caching if callee was not complete (due to recursion)
          isCurrentComplete &= isCalleeComplete;
        }
        // Found a potentialy cast, report true
        if (result) {
          // Cache and report even if it was incomplete
          // Missing traversal can never flip it to not found
          mayCastMap.insert(std::make_pair(F, true));
          *isComplete = true;
          return true;
        }
      }

      // No cast found anywhere, report false
      // Do not cache negative results if current traversal
      // was not complete (due to recursion)
      /*if (isCurrentComplete) {
        mayCastMap.insert(std::make_pair(F, false));
        }*/
      // Report to caller that this traversal was incomplete
      *isComplete = isCurrentComplete;
      return false;
    }

    bool isSafeStackFn(Function *F) {
      assert(F && "Function can't be null");

      std::set<Function*> visitedFunctions;
      bool tmp;
      bool mayCurrentCast = mayCast(&*F, visitedFunctions, &tmp);
      mayCastMap.insert(std::make_pair(&*F, mayCurrentCast));
      if (!mayCurrentCast)
        return false;

      return true;
    }

    void stackObjTracing(Module &M) {
      for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
        if(!HexTypeUtilSet->isInterestingFn(&*F))
          continue;
        // Apply stack optimization
        if (ClStackOpt && !isSafeStackFn(&*F))
          continue;

        handleFnPrameter(M, &*F);
        for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB)
          for (BasicBlock::iterator i = BB->begin(),
               ie = BB->end(); i != ie; ++i) {
            collectLifeTimeInfo(&*i);
            collectAllocaInstInfo(&*i);
          }
        findReturnInsts(&*F);
      }
      handleAllocaAdd(M);
      handleAllocaDelete(M);
    }

    void globalObjTracing(Module &M) {
      Function *FGlobal = setGlobalObjUpdateFn(M);
      BasicBlock *BBGlobal = BasicBlock::Create(M.getContext(),
                                                "entry", FGlobal);
      IRBuilder<> BuilderGlobal(BBGlobal);

      for (GlobalVariable &GV : M.globals()) {
        if (GV.getName() == "llvm.global_ctors" ||
            GV.getName() == "llvm.global_dtors" ||
            GV.getName() == "llvm.global.annotations" ||
            GV.getName() == "llvm.used")
          continue;

        if (HexTypeUtilSet->isInterestingType(GV.getValueType())) {
          StructElementInfoTy offsets;
          Value *NElems = NULL;
          Type *AllocaType;

          if(isa<ArrayType>(GV.getValueType())) {
            ArrayType *AI = dyn_cast<ArrayType>(GV.getValueType());
            AllocaType = AI->getElementType();
            NElems = ConstantInt::get(HexTypeUtilSet->Int64Ty,
                                      AI->getNumElements());
          }
          else {
            AllocaType = GV.getValueType();
            NElems = ConstantInt::get(HexTypeUtilSet->Int64Ty, 1);
          }

          HexTypeUtilSet->getArrayOffsets(AllocaType, offsets, 0);
          if(offsets.size() == 0) continue;

          HexTypeUtilSet->insertUpdate(&M, BuilderGlobal,
                                       "__update_global_oinfo",
                                       &GV, offsets, HexTypeUtilSet->DL.
                                       getTypeAllocSize(AllocaType),
                                       NElems, NULL, BBGlobal);
        }
      }
      BuilderGlobal.CreateRetVoid();
      appendToGlobalCtors(M, FGlobal, 0);
    }

    void emitTypeInfoAsGlobalVal(Module &M) {
      std::string mname = M.getName();
      HexTypeUtilSet->syncModuleName(mname);

      char ParentSetGlobalValName[MAXLEN];

      strcpy(ParentSetGlobalValName, mname.c_str());
      strcat(ParentSetGlobalValName, ".hextypepass_cinfo");

      HexTypeUtilSet->typeInfoArrayGlobal =
        HexTypeUtilSet->emitAsGlobalVal(M, ParentSetGlobalValName,
                        &(HexTypeUtilSet->typeInfoArray));

    }

    virtual bool runOnModule(Module &M) {
      // init HexTypePass
      CG = &getAnalysis<CallGraphWrapperPass>().getCallGraph();
      HexTypeLLVMUtil HexTypeUtilSetT(M.getDataLayout());
      HexTypeUtilSet = &HexTypeUtilSetT;
      HexTypeUtilSet->initType(M);

      HexTypeUtilSet->createObjRelationInfo(M);
      if (HexTypeUtilSet->AllTypeInfo.size() > 0)
        emitTypeInfoAsGlobalVal(M);

      // Init for only tracing casting related objects
      if (ClCastObjOpt || ClCreateCastRelatedTypeList)
        HexTypeUtilSet->setCastingRelatedSet();

      // Global object tracing
      globalObjTracing(M);

      // Stack object tracing
      stackObjTracing(M);
      return false;
    }
  };
}

//register pass
char HexType::ID = 0;
INITIALIZE_PASS_BEGIN(HexType, "HexType",
                      "HexTypePass: fast type safety for C++ programs.",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_END(HexType, "HexType",
                    "HexTypePass: fast type safety for C++ programs.",
                    false, false)
ModulePass *llvm::createHexTypePass() {
  return new HexType();
}
