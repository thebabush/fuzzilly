#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "../../common/fzl.h"

using namespace llvm;

namespace {
  struct BasicBlockInstrPass : public BasicBlockPass {
    static char ID;
    BasicBlockInstrPass() : BasicBlockPass(ID) {}

    Function* FzlFini         = nullptr;
    Function* FzlInit         = nullptr;
    Function* FzlTraceBB      = nullptr;
    Function* FzlTraceLoad    = nullptr;
    Function* FzlTraceStore   = nullptr;

    void createCtor(Module& M) {
      FzlInit->setDoesNotThrow();
      appendToGlobalCtors(M, FzlInit, 10000, nullptr);
    }

    void createDtor(Module& M) {
      FzlFini->setDoesNotThrow();
      appendToGlobalDtors(M, FzlFini, 10000, nullptr);
    }

    virtual bool doInitialization(Module& M) {
      errs() << "Init module: " << M.getName() << "\n";

      Type* Int32Ty = IntegerType::getInt32Ty(M.getContext());
      Type* Int64Ty = IntegerType::getInt64Ty(M.getContext());
      Type* VoidTy = Type::getVoidTy(M.getContext());

      FzlFini = cast<Function>(M.getOrInsertFunction(
          "fzl_fini",
          VoidTy
      ).getCallee());

      FzlInit = cast<Function>(M.getOrInsertFunction(
          "fzl_init",
          VoidTy
      ).getCallee());

      FzlTraceBB = cast<Function>(M.getOrInsertFunction(
          "fzl_trace_basic_block",
          VoidTy,
          Int32Ty
      ).getCallee());

      FzlTraceLoad = cast<Function>(M.getOrInsertFunction(
          "fzl_trace_memory_load",
          VoidTy,
          Int64Ty
      ).getCallee());

      FzlTraceStore = cast<Function>(M.getOrInsertFunction(
          "fzl_trace_memory_store",
          VoidTy,
          Int64Ty
      ).getCallee());

      createCtor(M);
      createDtor(M);

      return false;
    }

    void tagInstructionWithCounter(Instruction &I, unsigned Counter) {
      // TODO
    }

    void instrumentInstructions(BasicBlock &B) {
      // TODO: Move out
      Type* Int64Ty = IntegerType::getInt64Ty(B.getContext());

      unsigned Counter = 0;

      for (auto BI = B.begin(), BE = B.end(); BI != BE; ++BI, ++Counter) {
        Instruction& I = *BI;

        switch (I.getOpcode()) {
        case Instruction::Load: {
          Instruction* CastInst = PtrToIntInst::CreatePointerCast(((LoadInst*) &I)->getPointerOperand(), Int64Ty, "", &I);
          std::vector<Value*> args = {CastInst};
          CallInst::Create(FzlTraceLoad, args, "", &I);
          errs() << "LOAD:  " << I << "\n";
          break;
        }
        case Instruction::Store: {
          Instruction* CastInst = PtrToIntInst::CreatePointerCast(((LoadInst*) &I)->getPointerOperand(), Int64Ty, "", &I);
          std::vector<Value*> args = {CastInst};
          CallInst::Create(FzlTraceStore, args, "", &I);
          errs() << "STORE: " << I << "\n";
          break;
        }
        default:
          break;
        }
      }
    }

    bool runOnBasicBlock(BasicBlock &B) override {
      instrumentInstructions(B);

      auto I = B.getTerminator();
      auto md = I->getMetadata(fzl::TagBasicBlockIDKind);

      assert(md && "Found a basic block without ID :/");

      auto kBbID = cast<ConstantAsMetadata>(md->getOperand(0))->getValue();
      auto bbID = cast<ConstantInt>(kBbID);
      unsigned id = bbID->getValue().getLimitedValue();
      errs() << "Found basic block with ID: " << id << "\n";

      std::vector<Value*> args = {bbID};
      CallInst::Create(FzlTraceBB, args, "", B.getFirstNonPHI());

      return true;
    }
  };
} // namespace

char BasicBlockInstrPass::ID = 0;

static RegisterPass<BasicBlockInstrPass> X(
  "basic_block_instr",              // Command line argument
  "Add BB tracing instrumentation", // Command line desciption
  false,                            // Only looks at CFG
  false                             // Analysis Pass
);
