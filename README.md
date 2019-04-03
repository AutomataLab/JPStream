# JPStream

JPStream is a compilation system that compiles a given set of JSONPath queries into automata-based parallel executables with bounded memory footprints. First, it adapts a stream processing design that combines parsing and querying in one pass without generating any in-memory parse tree. This is achieved by jointly compile the query expressions and JSON syntax into a single automaton with two stacks. Further more, it offers a set of feasibility reasoning techniques to avoid path explosion during parallel execution. For more details about JPStream, please refer to our paper [1].


## Build

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


