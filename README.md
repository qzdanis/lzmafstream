# lzmafstream

C++ header-only library containing classes for reading and writing LZMA2 (.xz) files that are `std::istream` and `std:ostream` compatible. This library depends on `liblzma` and its associated headers.

## Requirements

This library is intended to be used on POSIX-compatible systems that support at least C++14. C++14 is required due to the library's usage of `std::make_unique`. The following system calls are required:

* `open`
* `read`
* `write`
* `close`
* `fstat`
* `mmap`
* `munmap`

Additionally, if included in code compiled with C++17 features, the libraries provide an additional overload for constructors and `open` calls that takes `const std::filesystem::path&`.

## Usage

### Reading

The interface for reading is the `lzma::ifstream` class. This classes memory-maps the input file into the program's address space and uses a configurable output buffer for the decompressed data. By default, this buffer is 1MiB, but can be changed with the second paramater to the `ifstream` constructor and `ifstream::open`.

### Writing

There are two available interfaces for writing files: `lzma::ofstream` and `lzma::mtofstream`. `mtofstream` uses a multithreaded stream encoder with a fixed number of threads as reported by `std::thread::hardware_concurrency()`. The second parameter to the constructors and `open` functions controls the `liblzma` preset (1-9, default 6). The third parameter controls the size of the data input and encoder output buffers. By default, these buffers are 1MiB.

For both reading and writing, progress made by `liblzma` can be retrieved with the `progress` member function. Progress reported by `lzma::mtofstream` may not be accurate, particularly when the stream is finished encoding.

The underlying streambuf classes, `ifbuf` and `ofbuf`, cannot be used directly.

## Caveats

The stream classes do not support all interfaces provided by `std::istream` and `std::ostream`. In particular, putback for `ifstream` is not implemented. Formatted input is untested, but may work. The `seek` and `tell` functions are likewise untested, but probably nonfunctional as the streambuf classes do not update `eback`, `gptr`, `egptr`, `pbase`, `pptr`, or `epptr`.
