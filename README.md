# JPStream

**JPStream** is a JSONPath query compilation system. It compiles a given a set of JSONPath queries into automata-based parallel executables with bounded memory footprints. First, it adopts a stream processing design that combines parsing and querying in one pass without generating any in-memory parse tree. This is achieved by jointly compiling the path expressions and JSON syntax into a single automaton with two stacks. Furthermore, it supports parallel processing of single large JSON file, enabled by a set of parallelization techniques customized to the stream processing automata. For more details about JPStream, please refer to our paper [1].


## Getting Started
### JSONPath
JSONPath is the basic query language of JSON data. It always refer to substructures of JSON data in the same way as XPath queries are used in combination with an XML document. The root object is always referred as `$`, since it is always anonymous. For more details, please refer [Stefan Goessner JsonPath implementation](https://goessner.net/articles/JsonPath/index.html#e2). 

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
| `[index1, index2, index3]`             | multiple array indexes      |
| `[-start:-end]`             | last few array elements      |
| `$..[*]`                       | get all arrays      |
| `()`                       | script expression, using underlying script engine   |

#### Path Examples

Given a simplified goggle route data

```javascript
{
    "routes": [ {
        "steps": [
            {
                "loc": {
                    "lat": 32,
                    "lng": -107
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
| `$.routes[*].steps[2].loc` |  location of the third step in each route  |
| `$.routes[0:2]` |  first two routes  |
| `$.routes[*].steps[?(@.loc)]` |  filter all steps of each route with location |
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

