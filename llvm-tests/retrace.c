// RUN: clang %s -O0 -S -emit-llvm -o %t.ll
// RUN: sed -i s/optnone//g %t.ll
// RUN: llvm-link %t.ll -S -o %t.ll
// RUN: opt -load=%bindir/passes/BasicBlockTagger/BasicBlockTaggerPass.so --basic_block_tagger %t.ll -S -o %t.meta.ll
// RUN: opt -load=%bindir/passes/BasicBlockInstr/BasicBlockInstrPass.so --basic_block_instr %t.meta.ll -S -o %t.tracer.ll
// RUN: clang++ -O0 %t.tracer.ll -o %t.tracer.bin %bindir/runtime/libfuzzilly.a
// RUN: export FZL_TRACE_FILE=%t.trace; echo -e "\x00" | %t.tracer.bin > %t.tracer.stdout; echo -n ""
// RUN: %bindir/tools/Retrace/fuzzilly-retrace -i %t.meta.ll -t %t.trace -o %t.retraced.bc
// RUN: opt -S %t.retraced.bc | %FileCheck %s
// RUN: clang -o %t.retraced.bin %t.retraced.bc
// RUN: echo A | %t.retraced.bin > %t.retraced.stdout; diff %t.retraced.stdout %t.tracer.stdout

#include <stdio.h>

// CHECK: main
int main() {
  // CHECK: fuzzilly_start_block
  // CHECK-NOT: store
  // CHECK: ret

  int a = getc(stdin);
  int b = 0;

  if (a == 0) {
    puts("ITSAME!");
  } else {
    puts("ITSANOTTAME!");
  }

  for (int i=0; i<3; i++) {
    a *= 2;
  }

  if (a > 1) {
    if (a > 2) {
      a = 2;
    } else {
      b = 3;
    }
  } else {
    a = 1;
  }

  return a + b;
}

