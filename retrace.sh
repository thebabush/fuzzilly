#!/bin/sh

./build/tools/Retrace/fuzzilly-retrace -i ./test/target.meta.ll -t ./test/trace.bin -o ./test/retraced.bc
llvm-dis-9 ./test/retraced.bc
clang-9 -o ./test/retraced ./test/retraced.bc

