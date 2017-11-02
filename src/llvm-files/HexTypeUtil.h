//===- HexTypeUtil.h - helper functions and classes for HexType ----*- C++-*-===//
////
////                     The LLVM Compiler Infrastructure
////
//// This file is distributed under the University of Illinois Open Source
//// License. See LICENSE.TXT for details.
////
////===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_HEXTYPE_H
#define LLVM_TRANSFORMS_UTILS_HEXTYPE_H

#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <set>
#include <list>

#define MAXNODE 1000000

#define STACKALLOC 1
#define HEAPALLOC 2
#define GLOBALALLOC 3
#define REALLOC 4
#define PLACEMENTNEW 5
#define REINTERPRET 6

#define CONOBJADD 1
#define VLAOBJADD 2
#define CONOBJDEL 3
#define VLAOBJDEL 4

namespace llvm {

  extern cl::opt<bool> ClStackOpt;
  extern cl::opt<bool> ClCastObjOpt;
  extern cl::opt<bool> ClSafeStackOpt;
  extern cl::opt<bool> ClCompileTimeVerifyOpt;
  extern cl::opt<bool> ClCreateCastRelatedTypeList;
  extern cl::opt<bool> ClInlineOpt;
  extern cl::opt<bool> ClMakeLogInfo;
  extern cl::opt<bool> ClMakeTypeInfo;

  typedef std::list<std::pair<uint64_t, StructType*>> StructElementInfoTy;
  typedef std::map<Function *, std::vector<Instruction *> *> FunctionReturnTy;

  class TypeDetailInfo {
  public:
    uint64_t TypeHashValue;
    uint32_t TypeIndex;
    std::string TypeName;
  };

  class TypeInfo {
  public:
    StructType *StructTy;
    TypeDetailInfo DetailInfo;
    uint32_t ElementSize;

    std::vector<TypeDetailInfo> DirectParents;
    std::vector<TypeDetailInfo> DirectPhantomTypes;
    std::vector<TypeDetailInfo> AllParents;
    std::vector<TypeDetailInfo> AllPhantomTypes;
  };

  class HexTypeCommonUtil {
  public:
    static uint64_t getHashValueFromStr(std::string &);
    static uint64_t getHashValueFromSTy(StructType *);
    static void syncTypeName(std::string &);
    static bool isInterestingStructType(StructType *);
    void updateCastingReleatedTypeIntoFile(Type *);
    bool isInterestingType(Type *);
    void writeInfoToFile(char *, char *);
  };

  class HexTypeLLVMUtil : public HexTypeCommonUtil {
  public:
    HexTypeLLVMUtil(const DataLayout &DL)
      : DL(DL) {
      }

    const DataLayout &DL;

    Type *VoidTy;
    Type *Int8PtrTy;
    Type *Int32PtrTy;
    Type *Int64PtrTy;
    Type *IntptrTyN;
    Type *IntptrTy;
    Type *Int128Ty;
    Type *Int64Ty;
    Type *Int32Ty;
    Type *Int8Ty;
    Type *Int1Ty;

    uint32_t AllTypeNum = 0;

    std::vector<TypeInfo> AllTypeInfo;
    std::vector<Constant*> typeInfoArray;
    std::vector<Constant*> typePhantomInfoArray;
    std::vector<uint64_t> typeInfoArrayInt;
    std::set<std::string> CastingRelatedSet;
    std::set<std::string> CastingRelatedExtendSet;

    GlobalVariable *typeInfoArrayGlobal;
    GlobalVariable *typePhantomInfoArrayGlobal;

    static void syncModuleName(std::string &);
    void initType(Module &);
    void createObjRelationInfo(Module &);
    Instruction *findNextInstruction(Instruction *);
    AllocaInst *findAllocaForValue(Value *);
    void emitRemoveInst(Module *, IRBuilder<> &, AllocaInst *);
    void getStructOffsets(StructType *, StructElementInfoTy &, uint32_t);
    void getArrayOffsets(Type *, StructElementInfoTy &, uint32_t);
    void insertUpdate(Module *, IRBuilder<> &, std::string, Value *,
                      StructElementInfoTy &, uint32_t , Value *,
                      Value *, BasicBlock *);
    void insertRemove(Module *, IRBuilder<> &, std::string, Value *,
                      StructElementInfoTy &, Value *, int , BasicBlock *);
    bool isInterestingFn(Function *);
    bool isSafeStackAlloca(AllocaInst *);
    void setCastingRelatedSet();
    void extendCastingRelatedTypeSet();
    GlobalVariable *getVerifyResultCache(Module &);
    GlobalVariable *getObjTypeMap(Module &);
    GlobalVariable *emitAsGlobalVal(Module &, char *, std::vector<Constant*> *);
    void getTypeInfoFromClang();

  private:
    bool VisitCheck[MAXNODE];
    void parsingTypeInfo(StructType *, TypeInfo &, uint32_t);
    void getDirectTypeInfo(Module &);
    void setTypeDetailInfo(StructType *, TypeDetailInfo &, uint32_t);

    void extendParentSet(int , int );
    void extendPhantomSet(int , int );
    void extendTypeRelationInfo();
    void sortSet(std::set<uint64_t> &);
    void getSortedAllParentSet();
    void getSortedAllPhantomSet();
    void removeNonCastingRelatedObj(StructElementInfoTy &);
    void emitInstForObjTrace(Module *, IRBuilder<> &, StructElementInfoTy &,
                             uint32_t , Value *, Value *, uint32_t , uint32_t,
                             uint32_t , Value *, BasicBlock *);
  };
} // llvm namespace
#endif  // LLVM_TRANSFORMS_UTILS_HEXTYPE_H
