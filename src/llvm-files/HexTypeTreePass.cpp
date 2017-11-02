//===-- HexTypeTreePass.cpp -----------------------------------------------===//
//
// This file is a part of HexType, a type confusion detector.
//
// The HexTypeTreePass has below two functions:
//   - Create object relationship information.
//   - Compile time verification
// This pass will run before all optimization passes run
// The rest is handled by the run-time library.
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
using namespace llvm;

namespace {
  struct HexTypeTree : public ModulePass {
    static char ID;
    HexTypeTree() : ModulePass(ID) {}

    virtual bool runOnModule(Module &M) {
      return false;
    }
  };
}

//register pass
char HexTypeTree::ID = 0;

INITIALIZE_PASS(HexTypeTree, "HexTypeTree",
                "HexTypePass: fast type safety for C++ programs.",
                false, false)

ModulePass *llvm::createHexTypeTreePass() {
  return new HexTypeTree();
}
