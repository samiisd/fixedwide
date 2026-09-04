# Compile-time and object size versus 0.4

Compiler: `clang version 22.1.8`
Flags: `-std=c++23 -O2 -c`. Median of 11 runs after one warmup.
Host: Linux 7.2.0-1-cachyos x86_64
Date: 2026-09-04T06:57:52+00:00

Each row compiles a translation unit that includes one header and
instantiates the work that header exists for, so the number covers
template instantiation and not only parsing.

| Include | 0.4 | this version | delta | object size 0.4 / now |
|---|---:|---:|---:|---|
| `fixed.hpp` | 23 ms | 30 ms | +30.4% | 936 / 936 bytes |
| `arithmetic.hpp` | 55 ms | 89 ms | +61.8% | 1136 / 1216 bytes |
| `chars.hpp` | 78 ms | 90 ms | +15.4% | 1264 / 1264 bytes |
