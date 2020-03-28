# gerrymandering/cpp

This is all the C++ code for hacking the election. It's organized like a typical C++ project, with `include` and `src` directories for all files written by us.

## Installation

`cd` into this directory, and perform the command:

```bash
cd cpp
make all
```
If you don't want to compile all binaries and only want a selection, the following binaries

### Dependencies

All dependencies are installed when using `make` or when installing through git submodule. They are:

- [rapidjson](https://github.com/Tencent/rapidjson)
- clipper

### Version

`c++11` is required to compile and run all code in this repository.