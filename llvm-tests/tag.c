// RUN: clang %s -O0 -S -emit-llvm -o %t.ll
// RUN: opt -load=%bindir/passes/BasicBlockTagger/BasicBlockTaggerPass.so --basic_block_tagger %t.ll -S -o %t.meta.ll
// RUN: cat %t.meta.ll | %FileCheck %s

#include <stdio.h>

int main() {
  // CHECK: fzl.BB_ID
  if (getc(stdin) > 0x20) {
    return 0;
  } else {
    return 1;
  }
}

