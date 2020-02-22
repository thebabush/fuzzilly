#!/bin/bash

LLVM_BINDIR=$(llvm-config-9 --bindir)

cd test

echo "====== COMPILING ======"
#$LLVM_BINDIR/clang -O1 -S -emit-llvm test.c
$LLVM_BINDIR/clang -O0 -S -emit-llvm complex.c -o test.ll
#sed -i s/optnone//g test.ll
#opt-9 -mem2reg test.ll -S > test_fix.ll
#mv test_fix.ll test.ll

echo "====== LLVM-LINK ======"
$LLVM_BINDIR/llvm-link ./test.ll -S -o target.ll

echo "====== TAG ======"
$LLVM_BINDIR/opt -load=../build/passes/BasicBlockTagger/BasicBlockTaggerPass.so --basic_block_tagger ./target.ll -S -o target.meta.ll

echo "====== INSTRUMENT ======"
$LLVM_BINDIR/opt -load=../build/passes/BasicBlockInstr/BasicBlockInstrPass.so --basic_block_instr ./target.meta.ll -S -o target.trace.ll

echo "====== LINK ======"
$LLVM_BINDIR/clang++ -O0 ./target.trace.ll -o target.trace.bin ../build/runtime/libfuzzilly.a

echo "====== MOTHAFUCKING RUNNING ======"
echo -e \\x00 | ./target.trace.bin

