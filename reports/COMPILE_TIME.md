# Compile-time and object size versus 0.4

Compiler: `clang version 22.1.8`
Flags: `-std=c++23 -O2 -c`. Median of 15 runs after one warmup.
Host: Linux 7.2.0-1-cachyos x86_64
Date: 2026-09-04T07:49:01+00:00

Each row compiles a translation unit that includes one header and
instantiates the work that header exists for, so the number covers
template instantiation and not only parsing.

| Include | 0.4 | this version | delta | object size 0.4 / now |
|---|---:|---:|---:|---|
| `fixed.hpp` | 20 ms | 26 ms | +30.0% | 936 / 936 bytes |
| `arithmetic.hpp` | 32 ms | 47 ms | +46.9% | 1136 / 1216 bytes |
| `chars.hpp` | 61 ms | 71 ms | +16.4% | 1264 / 1264 bytes |
