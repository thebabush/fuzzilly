# fuzzilly

![fuzzilly](https://user-images.githubusercontent.com/1985669/49517033-6d2bb380-f89b-11e8-8928-2f6668e3a133.png)

Fuzzilly is a PoC LLVM-based tool to *unroll* a program's execution.
It does so by instrumenting the LLVM IR of the target in such a way that during
execution it will dump its own memory accesses.
After that, it reconstructs the execution without any conditional statement.

It is source based.

I know the name is
[unfortunate](https://github.com/googleprojectzero/fuzzilli), but I made this
before that project was released.

Also, I made a beautiful logo.

Also, yes.

## Screenshots

Fuzzilly can turn this

![source](https://user-images.githubusercontent.com/1985669/75091570-0ad9d400-556f-11ea-8335-7fd0ec266255.png)

into this

![source-retraced](https://user-images.githubusercontent.com/1985669/75091569-09a8a700-556f-11ea-9fd5-40579fcb76dc.png)

or, if you prefer x86 assembly,

![asm](https://user-images.githubusercontent.com/1985669/75091564-06adb680-556f-11ea-96b6-0f51432dcd58.png)

into this

![asm-retraced](https://user-images.githubusercontent.com/1985669/75091566-07dee380-556f-11ea-9217-dd692cb44456.png)

## Usage

Have llvm-9 installed and then

```sh
./build.sh
cd build && make check && cd ..
./test.sh && ./retrace.sh && ./test/retraced
```

## Why?

Why not?

## Limitations

This is a PoC so it only works on simple programs.
Its main limitation is that it doesn't support varargs because LLVM treats them
in a weird way:
it has builtin IR operations made specifically for varargs, but it generates IR
that mixes them with normal memory accesses to the arguments.
Long story short: in order to support varargs on X86-64, one would need to
reimplement the SystemV ABI inside fuzzilly.

