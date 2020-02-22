#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Transforms/Utils/ValueMapper.h>
#include <llvm/Support/Format.h>

#include "Retracer.h"

#include "fzl/Retracer/TraceReader.h"

namespace fzl::Retracer {

Retracer::Retracer(llvm::LLVMContext *C, llvm::Module *M) {
  Context = C;
  Module = M;

  buildBasicBlockMap(BasicBlockIdMap, M);
}

llvm::Function *Retracer::buildTraceFromFile(const char *FN) {
  const char *SymFuzzillyMain = "fuzzilly_main";
  const char *SymOriginalMain = "main";

  TraceReader Reader(FN);

  if (!Reader.isOk()) {
    return nullptr;
  }

  llvm::Function *OrigFunk = Module->getFunction(SymOriginalMain);

  llvm::FunctionType *FT = Module->getFunction(SymOriginalMain)->getFunctionType();
  llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, SymFuzzillyMain, Module);

  llvm::BasicBlock *FatBlock = llvm::BasicBlock::Create(*Context, "fuzzilly_start_block", F);

  buildTrace(*OrigFunk, Reader, FatBlock);

  if (OptionSwapEntryPoints) {
    OrigFunk->setName(OptionSwappedEntryPointName);
    F->setName(SymOriginalMain);
  }

  return F;
}

void Retracer::buildBasicBlockMap(BasicBlockIdMapTy &Map, llvm::Module *Mod) {

  for (llvm::Function &F : *Mod) {
    for (auto BI = F.begin(), E = F.end(); BI != E; BI++) {
      llvm::BasicBlock *B = &(*BI);
      const llvm::Instruction *I = B->getTerminator();

      auto MD = I->getMetadata(fzl::TagBasicBlockIDKind);

      assert(MD && "Basic block without ID");

      auto MetadataConst = llvm::cast<llvm::ConstantAsMetadata>(MD->getOperand(0))->getValue();
      auto BBIntID = llvm::cast<llvm::ConstantInt>(MetadataConst);
      fzl::BasicBlockIdTy ID = BBIntID->getValue().getLimitedValue();

      Map[ID] = B;
    }
  }

}

void Retracer::buildTrace(llvm::Function &F, TraceReader &Reader, llvm::BasicBlock *OutBlock) {
  ArgVector Dummy;
  llvm::BasicBlock *NewOutBlock = nullptr;

  llvm::Value *RetVal = buildFunctionTrace(Dummy, F, Reader, OutBlock, &NewOutBlock);
  assert(NewOutBlock != nullptr && "RETRACER: buildFunctionTrace returned a null new basic block");

  // Append the return instruction
  OutBlock = NewOutBlock;
  llvm::ReturnInst::Create(F.getContext(), RetVal, OutBlock);
}

llvm::Value *Retracer::buildFunctionTrace(ArgVector &Args,
                                          llvm::Function &F,
                                          TraceReader &Reader,
                                          llvm::BasicBlock *OutBlock,
                                          llvm::BasicBlock **PtrNewOutBlock) {
  // Original instruction -> Unrolled instruction
  llvm::ValueToValueMapTy Remap;

  // Build the initial map using parent's Values
  remapFunctionArguments(F, Args, Remap);

  llvm::BasicBlock *PrevBlock = nullptr;
  while (std::optional<TraceEvent> MaybeEvent = Reader.getNext()) {
    TraceEvent Event = *MaybeEvent;

    assert(Event.Kind == TEK_BasicBlock && "RETRACER: Unexpected memory event");

    llvm::BasicBlock *OriginalBlock = BasicBlockIdMap[Event.BasicBlock.Id];

    llvm::errs() << "RETRACER: Processing block " << Event.BasicBlock.Id << "\n";
    llvm::errs() << *OriginalBlock;

    // Fix basic block
    for (llvm::Instruction &OriginalInstr : *OriginalBlock) {
      switch (OriginalInstr.getOpcode()) {
      case llvm::Instruction::Br: {
        // TODO(babush): Implement optional fake branching
        break;
      }
      case llvm::Instruction::Call: {
        OutBlock = handleCallInstruction(Reader, OutBlock, llvm::cast<llvm::CallInst>(OriginalInstr), Remap);
        break;
      }
      case llvm::Instruction::Load: {
        handleLoadInstruction(Reader, *OutBlock, llvm::cast<llvm::LoadInst>(OriginalInstr), Remap);
        break;
      }
      case llvm::Instruction::PHI: {
        handlePhiNode(PrevBlock, llvm::cast<llvm::PHINode>(OriginalInstr), Remap);
        break;
      }
      case llvm::Instruction::Ret: {
        return handleRetInstruction(PtrNewOutBlock, OutBlock, llvm::cast<llvm::ReturnInst>(OriginalInstr), Remap);
      }
      case llvm::Instruction::Switch: {
        handleSwitchInstruction(PrevBlock, llvm::cast<llvm::SwitchInst>(OriginalInstr), Remap);
        break;
      }
      case llvm::Instruction::Store: {
        handleStoreInstruction(Reader, *OutBlock, llvm::cast<llvm::StoreInst>(OriginalInstr), Remap);
        break;
      }
      default:handleGenericInstruction(*OutBlock, OriginalInstr, Remap);
      }
    }

    PrevBlock = OriginalBlock;
  }

  llvm_unreachable("RETRACER: End of function unrolling reached without RETs");
}

llvm::BasicBlock *Retracer::handleCallInstruction(TraceReader &Reader,
                                                  llvm::BasicBlock *OutBlock,
                                                  llvm::CallInst &OriginalCall,
                                                  llvm::ValueToValueMapTy &Remap) {
  // TODO(babush): Handle varargs
  if (OriginalCall.getCalledFunction()->isDeclaration()) {
    // We don't have the definition of this function
    // Just keep it and return the current basic block
    handleGenericInstruction(*OutBlock, OriginalCall, Remap);
    return OutBlock;
  }

  ArgVector ChildArgs;
  for (llvm::Value *A : OriginalCall.arg_operands()) {
    llvm::Value *MappedArg = Remap[A];

    if (MappedArg == nullptr) {
      assert(llvm::isa<llvm::Constant>(A) && "Calling a subfunction with an unmapped, not constant Value");
      ChildArgs.push_back(A);
    } else {
      ChildArgs.push_back(MappedArg);
    }
  }

  // In case the child created a new basic block
  llvm::BasicBlock *NewOutBlock;

  // Unroll the child
  llvm::Value
      *Ret = buildFunctionTrace(ChildArgs, *OriginalCall.getCalledFunction(), Reader, OutBlock, &NewOutBlock);
  Remap[&OriginalCall] = Ret;

  // Return the current basic block
  return NewOutBlock;
}

void Retracer::handleGenericInstruction(llvm::BasicBlock &OutBlock,
                                        const llvm::Instruction &OriginalInstr,
                                        llvm::ValueToValueMapTy &Map) {
  llvm::Instruction *NewInst = OriginalInstr.clone();
  llvm::RemapInstruction(NewInst, Map);
  OutBlock.getInstList().push_back(NewInst);
  Map[&OriginalInstr] = NewInst;
}

void Retracer::handleLoadInstruction(TraceReader &Reader,
                                     llvm::BasicBlock &OutBlock,
                                     llvm::LoadInst &LoadInst,
                                     llvm::ValueToValueMapTy &Map) {
  TraceEvent MemoryEvent = *Reader.getNext();
  assert(MemoryEvent.Kind == TEK_MemoryLoad && "Mismatch in event type!");

  MemoryAddressTy Address = MemoryEvent.MemoryAccess.Address;

  // We have the value in memory
  if (std::optional<llvm::Value *> Value = Memory.load(Address)) {
    Map[&LoadInst] = *Value;
    return;
  }

  // We don't have the value, keep the load instruction
  llvm::errs() << "RETRACER: Missing load from address " << llvm::format("%018p", Address) << "\n";
  handleGenericInstruction(OutBlock, LoadInst, Map);
}

void Retracer::handlePhiNode(const llvm::BasicBlock *PrevBlock,
                             const llvm::PHINode &OriginalPhi,
                             llvm::ValueToValueMapTy &Map) {
  assert(PrevBlock != nullptr && "PrevBlock is null");

  // We skip phi nodes and just keep track of their last value
  llvm::Value *OriginalValue = OriginalPhi.getIncomingValueForBlock(PrevBlock);
  Map[&OriginalPhi] = Map[OriginalValue];
}

llvm::Value *Retracer::handleRetInstruction(llvm::BasicBlock **PtrNewOutBlock,
                                            llvm::BasicBlock *OutBlock,
                                            const llvm::ReturnInst &RetInst,
                                            llvm::ValueToValueMapTy &Remap) {
  *PtrNewOutBlock = OutBlock;

  llvm::Value *RetVal = RetInst.getReturnValue();
  if (RetVal == nullptr) {
    return nullptr;
  }

  auto RI = Remap.find(RetVal);
  if (RI != Remap.end()) {
    return RI->second;
  }

  assert(llvm::isa<llvm::Constant>(RetVal) && "Returning a non-constant value with no mapping");
  return RetVal;
}

void Retracer::handleStoreInstruction(TraceReader &Reader,
                                      llvm::BasicBlock &OutBlock,
                                      llvm::StoreInst &Store,
                                      llvm::ValueToValueMapTy &Map) {
  TraceEvent MemoryEvent = *Reader.getNext();
  llvm::errs() << "EVENT TYPE: " << (int) MemoryEvent.Kind << "\n";
  llvm::errs() << Store << "\n";
  assert(MemoryEvent.Kind == TEK_MemoryStore && "Mismatch in event type!");

  Memory.dump(llvm::errs());

  llvm::Value *StoreValue = Store.getValueOperand();
  auto RI = Map.find(StoreValue);

  // We have the value in the Remap
  if (RI != Map.end()) {
    Memory.store(MemoryEvent.MemoryAccess.Address, RI->second);
    return;
  }

  // We are dealing with a constant
  if (auto *C = llvm::dyn_cast<llvm::Constant>(StoreValue)) {
    Memory.store(MemoryEvent.MemoryAccess.Address, C);
    return;
  }

  // We don't have the store value, so we keep the store instruction
  llvm::errs() << "RETRACER: Missing remap item for value " << *StoreValue << "\n";
  handleGenericInstruction(OutBlock, Store, Map);
}

void Retracer::handleSwitchInstruction(const llvm::BasicBlock *PrevBlock,
                                       const llvm::SwitchInst &SwitchInst,
                                       llvm::ValueToValueMapTy &Map) {
  // For now, do nothing
}

void Retracer::remapFunctionArguments(const llvm::Function &F, const ArgVector &Args, llvm::ValueToValueMapTy &Map) {
  unsigned ArgIdx = 0;
  size_t AS = Args.size();

  // Remap with early stop is parent's values are not enough
  for (auto AI = F.arg_begin(), AE = F.arg_end(); AI != AE && ArgIdx < AS; ++AI, ++ArgIdx) {
    Map[AI] = Args[ArgIdx];
  }
}

} // namespace fzl::Retracer
