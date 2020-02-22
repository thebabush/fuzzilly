#ifndef FUZILLY_RETRACE_H
#define FUZILLY_RETRACE_H

#include <unordered_map>

#include <llvm/IR/Module.h>

#include "fzl.h"
#include "TraceReader.h"
#include "MemoryModel.h"

namespace fzl::Retracer {

using BasicBlockIdMapTy = std::unordered_map<fzl::BasicBlockIdTy, llvm::BasicBlock *>;
using ArgVector = llvm::SmallVector<llvm::Value *, 10>;

class Retracer {
private:
  llvm::LLVMContext *Context;
  llvm::Module *Module;

  const bool OptionCreateFakeBranches = false;
  const bool OptionKeepMemoryAccesses = true;
  const bool OptionSwapEntryPoints = true;
  const char *OptionSwappedEntryPointName = "totally_not_main";

  BasicBlockIdMapTy BasicBlockIdMap;
  MemoryModel Memory;

public:
  Retracer(llvm::LLVMContext *C, llvm::Module *M);

  llvm::Function *buildTraceFromFile(const char *FN);

private:
  void buildBasicBlockMap(BasicBlockIdMapTy &Map, llvm::Module *Mod);
  llvm::Value *buildFunctionTrace(ArgVector &Args,
                                  llvm::Function &F,
                                  TraceReader &Reader,
                                  llvm::BasicBlock *OutBlock,
                                  llvm::BasicBlock **PtrNewOutBlock);
  void buildTrace(llvm::Function &F, TraceReader &Reader, llvm::BasicBlock *OutBlock);

  llvm::BasicBlock *handleCallInstruction(TraceReader &Reader,
                                                 llvm::BasicBlock *OutBlock,
                                                 llvm::CallInst &OriginalCall,
                                                 llvm::ValueToValueMapTy &Remap);

  static void handleGenericInstruction(llvm::BasicBlock &OutBlock,
                                       const llvm::Instruction &OriginalInstr,
                                       llvm::ValueToValueMapTy &Map);

  void handleLoadInstruction(TraceReader &Reader,
                             llvm::BasicBlock &OutBlock,
                             llvm::LoadInst &LoadInst,
                             llvm::ValueToValueMapTy &Map);

  static void handlePhiNode(const llvm::BasicBlock *PrevBlock,
                            const llvm::PHINode &OriginalPhi,
                            llvm::ValueToValueMapTy &Map);

  static llvm::Value *handleRetInstruction(llvm::BasicBlock **PtrNewOutBlock,
                                           llvm::BasicBlock *OutBlock,
                                           const llvm::ReturnInst &RetInst,
                                           llvm::ValueToValueMapTy &Remap);

  void handleStoreInstruction(TraceReader &Reader,
                              llvm::BasicBlock &OutBlock,
                              llvm::StoreInst &Store,
                              llvm::ValueToValueMapTy &Map);

  static void handleSwitchInstruction(const llvm::BasicBlock *PrevBlock,
                            const llvm::SwitchInst &SwitchInst,
                            llvm::ValueToValueMapTy &Map);

  static void remapFunctionArguments(const llvm::Function &F, const ArgVector &Args, llvm::ValueToValueMapTy &Map);

};

} // namespace fzl::Retracer

#endif //FUZILLY_RETRACE_H
