// RUN: clang %s -O0 -S -emit-llvm -o %t.ll
// RUN: opt -load=%bindir/passes/BasicBlockTagger/BasicBlockTaggerPass.so --basic_block_tagger %t.ll -S -o %t.meta.ll
// RUN: opt -load=%bindir/passes/BasicBlockInstr/BasicBlockInstrPass.so --basic_block_instr %t.meta.ll -S -o %t.trace.ll
// RUN: cat %t.trace.ll | %FileCheck %s

#include <stdio.h>

int main() {
  // CHECK: fzl_trace_basic_block
  if (getc(stdin) > 0x20) {
    return 0;
  } else {
    return 1;
  }
}

