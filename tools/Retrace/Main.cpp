#include <fstream>
#include <unordered_map>

#include <llvm/ADT/STLExtras.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/InitializePasses.h>
#include <llvm/Pass.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Transforms/Utils/ValueMapper.h>

#include "../../common/fzl.h"
#include "fzl/Retracer/Retracer.h"


int main(int argc, char *argv[]) {
  llvm::cl::opt<bool> OutputAssembly(
      "S",
      llvm::cl::desc("Write output as LLVM assembly")
  );

  llvm::cl::opt<std::string> IRFilename(
      "i",
      llvm::cl::desc("Specify IR filename"),
      llvm::cl::value_desc("filename"),
      llvm::cl::Required
  );

  llvm::cl::opt<std::string> OutFilename(
      "o",
      llvm::cl::desc("Specify output filename"),
      llvm::cl::value_desc("filename"),
      llvm::cl::Required
  );

  llvm::cl::opt<std::string> TraceFilename(
      "t",
      llvm::cl::desc("Specify trace.bin filename"),
      llvm::cl::value_desc("filename"),
      llvm::cl::Required
  );

  llvm::cl::ParseCommandLineOptions(argc, argv);

  static llvm::LLVMContext Context;
  llvm::SMDiagnostic Diag;
  std::unique_ptr<llvm::Module> Mod = llvm::parseIRFile(IRFilename, Diag, Context);

  if (Mod == nullptr) {
    llvm::errs() << "Invalid file " << IRFilename << "\n";
    return 1;
  }

  fzl::Retracer::Retracer R(&Context, Mod.get());
  llvm::Function *TracedFunction = R.buildTraceFromFile(TraceFilename.c_str());

  // Debug print
  llvm::errs() << *TracedFunction;

  // Write bitcode to file
  std::error_code EC;
  llvm::raw_fd_ostream OS(OutFilename, EC);

  if (OutputAssembly) {
    llvm::createPrintModulePass(OS, "", true)->runOnModule(*Mod);
  } else {
    llvm::WriteBitcodeToFile(*Mod, OS);
  }

  return 0;
}

