# JPStream

**JPStream** is a JSONPath query compilation system. It compiles a given a set of JSONPath queries into automata-based parallel executables with bounded memory footprints. First, it adopts a stream processing design that combines parsing and querying in one pass without generating any in-memory parse tree. This is achieved by jointly compiling the path expressions and JSON syntax into a single automaton with two stacks. Furthermore, it supports parallel processing of single large JSON file, enabled by a set of parallelization techniques customized to the stream processing automata. For more details about JPStream, please refer to our paper [1].

## Publication
[1] Lin Jiang, Xiaofan Sun, Umar Farooq, and Zhijia Zhao. Stream Processing of Contemporary Semi-Structured Data on Commodity Parallel Processors - A Compilation-based Approach. In Proceedings of the 24th ACM International Conference on Architectural Support for Programming Languages and Operating Systems (ASPLOS), 2019.

## Getting Started
### JSONPath
JSONPath is the basic query language of JSON data. It refers to substructures of JSON data in a similar way as XPath queries are used for XML data. For the details of JSONPath syntax, please refer to [Stefan Goessner's article](https://goessner.net/articles/JsonPath/index.html#e2). 

#### Supported Operators
| Operator                  |   Description     |
| :-----------------------: |:-----------------:|
| `$`                       | root object              |
| `@`                       | current object filtered by predicate      |
| `.`                       | child object      |
| `[]`                       | child array      |
| `*`                       | wildcard, all objects or array members          |
| `..`                      | decendant elements |
| `[index]`             | array index      |
| `[start:end]`             | array slice operator      |
| `[?(<expression>)]`       | filter expression for evaluation |

#### Operators Not Supported
| Operator                  |   Description     |
| :-----------------------: |:-----------------:|
| `[index1, index2, ...]`             | multiple array indexes      |
| `[-start:-end]`             | last few array elements      |
| `$..[*]`                       | get all arrays      |
| `()`                       | script expression, using underlying script engine   |

#### Path Examples
Consider a piece of simplified Google route data
```javascript
{
    "routes": [ {
        "steps": [
            {
                "loc": {
                    "lat": 32,
                    "lng": -107
                },
                "dis": {
                    "text": "92 ft",
                    "value": 28
                }
            },
            {
                "loc": {
                    "lat": 35,
                    "lng": -106
                }
            }
        ] }
    ]
}
```
| JsonPath | Result |
| :------- | :----- |
| `$.routes[*].steps[*]` | all steps of each route     |
| `$.routes[*].steps[*].*` | all things in steps of each route     |
| `$..loc`| all locations                         |
| `$.routes[*]..loc` |  location in each route  |
| `$.routes[*].steps[2].dis` |  distance of the third step in each route  |
| `$.routes[0:2]` |  first two routes  |
| `$.routes[*].steps[?(@.loc&&@.dis)]` |  filter all steps of each route with location and distance |
| `$.routes[*].steps[?(@.loc.lat==32)]` |  filter all steps of each route with location at 32 degrees latitude |
| `$..*` |  everything in JSON structure |


### Build

There are some requirements for building this system: cmake `3.12+` and `gcc 4.8.5`. 
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
There are several running cases in "query_test.c" under the folder demo/. New cases can be added there. To run it, after building the system, execute the following commands: 
```
cd build/bin
./query_test
```
### Demo
For serial execution:
```c
    char* file_path = "../../dataset/bb.json";
    PathProcessor* path_processor = createPathProcessor("$.root.products[*].categoryPath[1:3]");
    Output* output = serialRun(path_processor, file_path);
```
For parallel execution:
```c
    char* file_path = "../../dataset/bb.json";
    PathProcessor* path_processor = createPathProcessor("$.root.products[*].categoryPath[1:3]");
    int num_core = 16;
    Output* output = parallelRun(path_processor, file_path, num_core);
```

For parallel execution with data constraint learning (more efficient): 
```c
    PathProcessor* path_processor = createPathProcessor("$.root.products[*].categoryPath[1:3]");
    
    //collecting data constraints is optional, but can often make parallel execution more efficient
    char* train_file_path = "../../dataset/bb.json";
    ConstraintTable* ct = collectDataConstraints(path_processor, train_file_path);
    
    //parallel exeuction
    char* input_file_path = "../../dataset/bb.json";
    int num_core = 16;
    Output* output = parallelRunOpt(path_processor, input_file_path, num_core, ct);
```

## Internal API
### JSONPath Parser
- `ASTNode* analysisJSONPath(const char* data)`: parse the json path string into an AST format
- `void printJsonPathAST(JSONPathNode* root)`: print the tree structure

### DFA Builder
- `JSONQueryDFA* buildJSONQueryDFA(const char* json_path, JSONQueryDFAContext* context)`: Create a JSON Query DFA from a JSON Path string
- `JSONQueryDFA* buildJSONQueryDFAFromAST(JSONPathNode* json_path, JSONQueryDFAContext* context)`: Create a JSON Query DFA from a JSON Path AST 

- `JSONPathNode* getContextSubtree(JSONQueryDFAContext* ctx, int stop_state)`: get the subtree that a stop state is connected
-  `int getContextSizeOfMapping(JSONQueryDFAContext* ctx, int stop_state)`: get the size of mapping 
- `int getContextValueOfMapping(JSONQueryDFAContext* ctx, int stop_state, int idx)`: get the content of one mapping. Idx is the number from range (0 ~ size-1) that you get from the previous API.

### Serial Streaming Automaton
- `void initStreamingAutomaton(StreamingAutomaton* sa, JSONQueryDFA* qa)`: Initialize streaming automaton based on query automaton. 
- `void destroyStreamingAutomaton(StreamingAutomaton* sa)`: Free dynamic memory spaces allocated by streaming automaton. 
- `void executeAutomaton(StreamingAutomaton* sa, char* json_stream, int data_constraint_flag)`: Execute streaming automaton based on input stream. To generate data constraint table, `data_constraint_flag` should be `OPEN` (otherwise it should be `CLOSE`). 

### File Loading and Partitioning
- `char* loadInputStream(char* file_name)`: Load input stream into memory.
- `char* loadBoundedInputStream(char* file_name, int* start_pos)`: Load the next available input chunk with bounded memory footprint, `start_pos` records the starting position of the next available chunk, the initial value is 0.
- `PartitionInfo partitionInputStream(char* input_stream, int num_core)`: Split the loaded input stream into several chunks.
- `PartitionInfo partitionFile(char* file_name, int num_core)`: Load and partition input stream into several chunks.

### Parallel Streaming Automata
- `void initParallelAutomata(ParallelAutomata* pa, JSONQueryDFA* qa)`: Initialize parallel streaming automata based on query automaton. 
- `void destroyParallelAutomata(ParallelAutomata* pa)`: Free dynamic memory spaces allocated by parallel streaming automata. 
- `void executeParallelAutomata(ParallelAutomata* pa, PartitionInfo par_info, ConstraintTable* ct)`: Execute parallel streaming automata based on partitioned chunks. Data constraint table can be used for runtime optimization. 

### Predicate Filtering
- `void initPredicateFilter(PredicateFilter* pf, TupleList* tl, JSONQueryDFAContext* ctx, char* input_stream)`: Initialize predicate filter based on 2-tuple list and query automaton. 
- `void destroyPredicateFilter(PredicateFilter* pf)`: Free dynamic memory spaces allocated by predicate filtering component. 
- `Output* generateFinalOutput(PredicateFilter* pf)`: Run predicate filter component and generate final output list. 

## Usage of Internal API
### Loading Input Stream
To load input stream without partitioning:
```c
    //load input stream
    char* stream = loadInputStream("../../dataset/wiki.json");
    
    //load input stream with bounded memory
    int start_pos = 0;
    char* stream = loadInputStream("../../dataset/wiki.json", &start_pos);
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
    executeParallelAutomata(&pa, pInfo, NULL);
    
    //run parallel automata with data constraint learning
    executeParallelAutomata(&pa, pInfo, sa.constraint_table);
    
    //get results generated from parallel automata
    TupleList* tl = pa.tuple_list;
```
### Filtering Results for JSONPath with Predicates
```c
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx, stream);
    Output* final = generateFinalOutput(&pf);
```

