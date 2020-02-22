#ifndef FZL_FZL_H
#define FZL_FZL_H

#include <cstdint>

#define FZL_DEV

#ifdef FZL_DEV
#include <stdio.h>
#define FZL_DBG(...) fprintf(stderr, __VA_ARGS__)
#else
#define FZL_DBG(...) {}
#endif

namespace fzl {

using BasicBlockIdTy = uint32_t;
using MemoryAddressTy = uint64_t;

// Trace tags
extern const char *TagBasicBlockIDKind;
extern const char *TagLoadStoreInstructionKind;

enum {
  TEK_BasicBlock    = 0,
  TEK_MemoryLoad    = 1,
  TEK_MemoryStore   = 2,
};

struct BasicBlockEvent {
  BasicBlockIdTy Id;
};

struct MemoryAccessEvent {
  MemoryAddressTy Address;
};

struct TraceEvent {
  uint8_t Kind;
  union {
    BasicBlockEvent BasicBlock;
    MemoryAccessEvent MemoryAccess;
  };
};

} // namespace

#endif
