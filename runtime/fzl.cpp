#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../common/fzl.h"

extern "C" {
void fzl_init();
void fzl_fini();
void fzl_trace_basic_block(fzl::BasicBlockIdTy Id);
void fzl_trace_memory_load(fzl::MemoryAddressTy Addr);
void fzl_trace_memory_store(fzl::MemoryAddressTy Addr);
}

namespace {
int TraceFD = -1;
} // namespace

void fzl_init() {
  const char* traceName = getenv("FZL_TRACE_FILE");
  if (traceName == nullptr) {
    traceName = "trace.bin";
  }
  TraceFD = open(traceName, O_CREAT | O_WRONLY | O_TRUNC, 0664);
  if (TraceFD == -1) {
    fputs("FZL: Couldn't open trace file!\n", stderr);
  }
}

void fzl_fini() {
  close(TraceFD);
}

void fzl_trace_basic_block(fzl::BasicBlockIdTy Id) {
  FZL_DBG("FZL_TRACE_BASIC_BLOCK:  %18d\n", Id);
  uint8_t Tag = fzl::TEK_BasicBlock;
  write(TraceFD, (void *) &Tag, sizeof(Tag));
  write(TraceFD, (void *) &Id,  sizeof(Id));
}

void fzl_trace_memory_access(uint8_t Tag, fzl::MemoryAddressTy Addr) {
  write(TraceFD, (void *) &Tag,    sizeof(Tag));
  write(TraceFD, (void *) &Addr,   sizeof(Addr));
}

void fzl_trace_memory_load(fzl::MemoryAddressTy Addr) {
  FZL_DBG("FZL_TRACE_MEMORY_LOAD:  %018p\n", Addr);
  uint8_t Tag = fzl::TEK_MemoryLoad;
  fzl_trace_memory_access(Tag, Addr);
}

void fzl_trace_memory_store(fzl::MemoryAddressTy Addr) {
  FZL_DBG("FZL_TRACE_MEMORY_STORE: %018p\n", Addr);
  uint8_t Tag = fzl::TEK_MemoryStore;
  fzl_trace_memory_access(Tag, Addr);
}

