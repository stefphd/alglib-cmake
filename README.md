# Alglib cmake

Set of CMake files to download and build [ALGLIB](http://www.alglib.net/) as a standalone project. 
From the [ALGLIB](http://www.alglib.net/), here a description of the library:
> ALGLIB is a cross-platform numerical analysis and data processing library. It supports several programming languages (C++, C#, Delphi) and several operating systems (Windows and POSIX, including Linux). 
ALGLIB features include:
> - Data analysis (classification/regression, statistics)
> - Optimization and nonlinear solvers
> - Interpolation and linear/nonlinear least-squares fitting
> - Linear algebra (direct algorithms, EVD/SVD), direct and iterative linear solvers
> - Fast Fourier Transform and many other algorithms

## Build and install

```bash
$ git clone https://github.com/stefphd/alglib-cmake
$ cd alglib-cmake
$ mkdir build && cd build
$ cmake ..
$ cmake --build . --config Release --target alglib
$ cmake --build . --config Release --target minnlc_d_sparse
$ cmake --install . --prefix path/to/install/dir
```

You can either build a static or a shared (dynamic) library (option `BUILD_SHARED_LIBS`). A test example `minnlc_d_sparse` is also built; see the [ALGLIB doc](https://www.alglib.net/translator/man/manual.cpp.html#example_minnlc_d_sparse).

### Dynamic library in Windows

To enable the possibility to link a dynamic library version of ALGLIB in Windows, the library is built with the `CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=ON` option to export all symbols. However, global data symbols need to be declared with `__declspec(dllimport)` when consuming the library; see [CMake doc](https://cmake.org/cmake/help/latest/prop_tgt/WINDOWS_EXPORT_ALL_SYMBOLS.html). This would require changes in the ALGLIB headers, which is quite annoying. 

The current workaround is to create local instances of the required symbols, and use them in the code instead of the default ones. This is shown in the test example.

Note that this works also when the static library is linked, altough not strictly necessary.

## License 
alglib-cmake is licensed under either the GNU Lesser General Public License v3.0 : 

https://www.gnu.org/licenses/lgpl-3.0.html

or the GNU Lesser General Public License v2.1 :

https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html

at your option.
