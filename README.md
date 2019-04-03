# JPStream

JPStream is a compilation system that compiles a given set of JSONPath queries into automata-based parallel executables with bounded memory footprints. First, it adapts a stream processing design that combines parsing and querying in one pass without generating any in-memory parse tree. This is achieved by jointly compile the query expressions and JSON syntax into a single automaton with two stacks. Further more, it offers a set of feasibility reasoning techniques to avoid path explosion during parallel execution. For more details about JPStream, please refer to our paper [1].


## Getting Started
### Install and Build

To build the project, we need CMake for makefile generator.

```
mkdir build
cd build
cmake ..
make
```

If you would like a release build:

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
### Demo

## Example Usage

## Publications
[1] Lin Jiang, Xiaofan Sun, Umar Farooq, and Zhijia Zhao. Stream Processing of Contemporary Semi-Structured Data on Commodity Parallel Processors - A Compilation-based Approach. In Proceedings of the 24th ACM International Conference on Architectural Support for Programming Languages and Operating Systems (ASPLOS), 2019.

