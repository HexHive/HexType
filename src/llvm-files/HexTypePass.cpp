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

#include "llvm/Pass.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {

  struct HexType : public ModulePass {
    static char ID;
    HexType() : ModulePass(ID) {}

    bool doInitialization(Module &M) {
      return true;
    }

    bool doFinalization(Module &M) {
      return false;
    }

    virtual bool runOnModule(Module &M) {
      return false;
    }
  };
}

//register pass
char HexType::ID = 0;

INITIALIZE_PASS(HexType, "HexType",
                "HexTypePass: fast type safety for C++ programs.",
                false, false)

ModulePass *llvm::createHexTypePass() {
  return new HexType();
}


