# An optimization model to optimally plan the renewal of an aircraft fleet 

This implementation supplements the article

[Manke, L.-M.; Joormann, I. "Aircraft Fleet Planning: An Optimization Model with Integrated CO$_2$ Trading Systems" ]

## Dependencies

This code depends on the following packages:

- A `C++` compiler supporting `C++ 17`, such as [gcc](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/)
- [CMake](https://cmake.org/) `>= 3.22.1`
- [Boost](https://www.boost.org/) `>= 1.74`
- [SCIP](https://scipopt.org/) `>= 7.0.3`
- [GRAPHVIZ](https://graphviz.org/) `>= 2.42.2`, optional package to print graph

## Build

The following commands build the code when executed in the
root folder of the project:

- `mkdir build`
- `cd build`
- `cmake ..`
- `make`

**Beware:** To obtain correct (i.e. comparable) timing results, ensure
the following:

- Instruct `cmake` to compile optimized code by setting an appropriate
  [build type](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html).
- Use [Gurobi](https://gurobi.com/) as a back end for SCIP.
- Ensure that SCIP itself is compiled in release mode (this *should* be the case by default)

## Instances

The instances were generated, as mentioned in the article, using a non-deterministic 
random number generator. Our generated instances are deposited inside test_instances.zip 
and can be reoptimized via loading the orig.lp file into the scip program. 
New instances will be generated when running the program.

