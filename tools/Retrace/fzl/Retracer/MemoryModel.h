#ifndef FUZILLY_MEMORY_H
#define FUZILLY_MEMORY_H

#include <map>

#include <llvm/IR/Value.h>

#include "fzl.h"

namespace fzl {
namespace Retracer {

using MemoryContent = std::pair<unsigned, llvm::Value*>;
using MemoryMap = std::map<std::uint64_t, MemoryContent>;

class MemoryModel {
private:
  MemoryMap Memory;

public:
  std::optional<llvm::Value*> load(MemoryAddressTy Address);
  void store(MemoryAddressTy Address, llvm::Value *Value);

  void dump(llvm::raw_ostream& out);
};

} // namespace Retracer
} // namespace fzl

#endif //FUZILLY_MEMORY_H
