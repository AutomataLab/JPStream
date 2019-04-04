# JPStream

**JPStream** is a compilation system that compiles a given set of JSONPath queries into automata-based parallel executables with bounded memory footprints. First, it adapts a stream processing design that combines parsing and querying in one pass without generating any in-memory parse tree. This is achieved by jointly compile the query expressions and JSON syntax into a single automaton with two stacks. Furthermore, it offers a set of feasibility reasoning techniques to avoid path explosion during parallel execution. For more details about JPStream, please refer to our paper [1].


## Getting Started
### Build

There are some requirements for building this system: cmake `3.12+` and `gcc 4.8.5`. 
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
### Demo

There are several running cases in "query_test.c" under the folder demo/. New cases can be added there. To run it, after building the system, execute the following commands: 
```
cd build/bin
./query_test
```

## Example Usage
### Loading Input Stream
To load input stream without partitioning:
```c
    char* stream = loadJSONStream("../../dataset/wiki.json");
```
To load input stream with partitioning:
```c
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
```
### Generating Deterministic Finite Automaton (DFA) for JSONPath query
```c
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
```
### Getting Executables and Run
For serial streaming automaton,
```c
    //create streaming automaton
    StreamingAutomaton sa;
    initStreamingAutomaton(&sa, dfa);

    //run streaming automaton without generating data constraints
    executeAutomaton(&sa, stream, CLOSE);
    
    //run streaming automaton and generate data constraints
    executeAutomaton(&sa, stream, OPEN);
    
    //get results generated from streaming automaton
    TupleList* tl = sa.tuple_list;
```
For parallel streaming automata,
```c
    //create parallel streaming automata 
    ParallelAutomata pa;
    initParallelAutomata(&pa, dfa);
    
    //run parallel automata without data constraint learning
    executeParallelAutomata(&pa, pInfo, WARMUP, NULL);
    
    //run parallel automata with data constraint learning
    executeParallelAutomata(&pa, pInfo, WARMUP, sa.constraint_table);
    
    //run parallel automata without warming up CPU
    executeParallelAutomata(&pa, pInfo, NOWARMUP, NULL);
    
    //get results generated from parallel automata
    TupleList* tl = pa.tuple_list;
```
### Filtering Results for JSONPath with Predicates
```c
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
```

## Publications
[1] Lin Jiang, Xiaofan Sun, Umar Farooq, and Zhijia Zhao. Stream Processing of Contemporary Semi-Structured Data on Commodity Parallel Processors - A Compilation-based Approach. In Proceedings of the 24th ACM International Conference on Architectural Support for Programming Languages and Operating Systems (ASPLOS), 2019.

