# cppmpiler

This is my implementation of [Writing An Interpreter In Go](https://interpreterbook.com/)
and [Writing A Compiler In Go](https://compilerbook.com/) in cpp.

## Development Setup

For setting up the development environment. You must develop in the UNIX-like environment with the following requirements:

+ Latest `gcc` or `clang` for compilation.
+ Latest `cmake` for building the project.
+ `clang-format` for formatting the code.
+ `Doxygen` and `graphviz` for building the docs.

## Build

It is easy to build for this project:

```sh
git clone https://github.com/shejialuo/cppmpiler
cd cppmpiler && mkdir build && cd build
cmake ..
make -j12
```

## Documentation

You could look at [docs](https://shejialuo.github.io/cppmpiler/) for documentation.
