# Compile-time and object size versus 0.4

Compiler: `clang version 22.1.8`
Flags: `-std=c++23 -O2 -c`. Median of 15 runs after one warmup.
Host: Linux 7.2.0-1-cachyos x86_64
Date: 2026-09-04T07:23:21+00:00

Each row compiles a translation unit that includes one header and
instantiates the work that header exists for, so the number covers
template instantiation and not only parsing.

| Include | 0.4 | this version | delta | object size 0.4 / now |
|---|---:|---:|---:|---|
| `fixed.hpp` | 21 ms | 27 ms | +28.6% | 936 / 936 bytes |
| `arithmetic.hpp` | 33 ms | 48 ms | +45.5% | 1136 / 1216 bytes |
| `chars.hpp` | 63 ms | 72 ms | +14.3% | 1264 / 1264 bytes |
