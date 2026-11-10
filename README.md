# gcmalloc
**gcmalloc** - precision GC for C.

## Description
**gcmalloc** is a precision GC for C, written in C.

This is a lowercase version of [GCMalloc](https://github.com/TheCRatTime/GCMalloc/).
Warning: GCMalloc is older than gcmalloc, use it instead.

### Features
- **No memory leaks**: [dbg_alloc](https://github.com/TheCRatTime/dbg_alloc/)
  shows all blocks are freed without any corruption (`tests/gc_test.c`).
- **Mark-and-Sweep GC**: one pause, fastest program work.
- **C89**/**ANSI C**: compiles everywhere.
- **Simple API**: just **GC_INIT()** and **GC_DEINIT()** in every function,
  or use root frames.

### Building
Needed:
- C Compiler
- CMake 3.10 and greater
- GNU Make

```bash
cmake -B build
cd build/
make
```

### Authors
TheCRatTime

### License
Project is under Apache 2.0 license
