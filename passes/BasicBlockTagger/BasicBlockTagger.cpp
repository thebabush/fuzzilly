#include <cstdint>

#include "llvm/IR/Constants.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "../../common/fzl.h"

using namespace llvm;

namespace {
  struct BasicBlockTaggerPass : public ModulePass {
    static char ID;
    BasicBlockTaggerPass() : ModulePass(ID) {}

    void tagBasicBlock(BasicBlock& BB, fzl::BasicBlockIdTy id) {
      auto &ctx = BB.getContext();
      auto I = BB.getTerminator();
      MDNode* node = MDNode::get(ctx, ConstantAsMetadata::get(ConstantInt::get(
        ctx, APInt(8 * sizeof(fzl::BasicBlockIdTy), id, false)
      )));
      I->setMetadata(fzl::TagBasicBlockIDKind, node);
    }

    bool runOnModule(Module &M) {
      fzl::BasicBlockIdTy counter = 0;

      for (auto& F : M) {
        for (auto& BB : F) {
          errs() << "I saw a BB: " << counter << "\n";
          tagBasicBlock(BB, counter);
          ++counter;
        }
      }

      // We modified the module
      return true;
    }
  };
} // namespace

char BasicBlockTaggerPass::ID = 0;

static RegisterPass<BasicBlockTaggerPass> X(
  "basic_block_tagger",                  // Command line argument
  "Tag every basic block in the target", // Command line desciption
  false,                                 // Only looks at CFG
  false                                  // Analysis Pass
);

