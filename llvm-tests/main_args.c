// RUN: clang %s -O0 -S -emit-llvm -o %t.ll
// RUN: sed -i s/optnone//g %t.ll
// RUN: llvm-link %t.ll -S -o %t.ll
// RUN: opt -load=%bindir/passes/BasicBlockTagger/BasicBlockTaggerPass.so --basic_block_tagger %t.ll -S -o %t.meta.ll
// RUN: opt -load=%bindir/passes/BasicBlockInstr/BasicBlockInstrPass.so --basic_block_instr %t.meta.ll -S -o %t.tracer.ll
// RUN: clang++ -O0 %t.tracer.ll -o %t.tracer.bin %bindir/runtime/libfuzzilly.a
// RUN: export FZL_TRACE_FILE=%t.trace; %t.tracer.bin arg > %t.tracer.stdout
// RUN: %bindir/tools/Retrace/fuzzilly-retrace -S -i %t.meta.ll -t %t.trace -o %t.retraced.ll
// RUN: clang -o %t.retraced.bin %t.retraced.ll
// RUN: %t.retraced.bin > %t.retraced.stdout; diff %t.retraced.stdout %t.tracer.stdout

#include <stdio.h>

int child(int arg) {
  return arg - 1;
}

int main(int argc, char *argv[]) {
  argc = child(argc);

  if (argc == 1) {
    puts("ONE");
  } else {
    puts("NOT ONE");
  }

  return 0;
}

