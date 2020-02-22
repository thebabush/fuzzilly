#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Format.h>

#include "MemoryModel.h"

namespace fzl {
namespace Retracer {

std::optional<llvm::Value*> MemoryModel::load(MemoryAddressTy Address) {
  // TODO: Handle overlapping
  auto Ret = Memory.find(Address);
  return Ret == Memory.end() ? std::optional<llvm::Value*>{} : std::make_optional(Ret->second.second);
}

void MemoryModel::store(MemoryAddressTy Address, llvm::Value *Value) {
  // TODO: Handle overlapping
  llvm::Type* ValueType = Value->getType();
  if (auto Integer = llvm::dyn_cast<llvm::IntegerType>(ValueType)) {
    unsigned int Bits = Integer->getScalarSizeInBits();
    llvm::errs() << "STORE BITS: " << Bits << "\n";
//    if (Bits != 32) return;
    Memory[Address] = std::make_pair(Bits, Value);
  }
}

void MemoryModel::dump(llvm::raw_ostream &out) {
  out << "### MEMORY DUMP BEG ###\n";
  for (auto&[K, V]: Memory) {
    out << llvm::format("%018p: ", K) << V.first << "\n";
  }
  out << "### MEMORY DUMP END ###\n";
}

} // namespace Retracer
} // namespace fzl
