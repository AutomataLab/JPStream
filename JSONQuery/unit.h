#ifndef __UNIT_H__
#define __UNIT_H__

#include "stack_p.h"
#include "dfa_builder.h"
#include "tuple_list.h"

#define MAX_STATE 100  //maximum query states
#define MAX_UNMATCHED_SYMBOL 100 //maximum unmatched symbols
#define UNIT_UNMATCHED 1 //unit starts with unmatched symbol
#define UNIT_MATCHED 2   //finish handling unmatched symbol
#define MAX_UNIT 100 //maximum number of units
#define MAX_CANDIDATE_GROUP 1

typedef struct UnmatchedSymbol{
    int token_type;
    int position;
}UnmatchedSymbol;

typedef struct Unit
{
    //starting querying states
    int starting_states[MAX_STATE]; 
    int count_start_states;
    //record the token type of unmatched symbols
    int unmatched_symbols[MAX_UNMATCHED_SYMBOL];  
    int count_unmatched_symbols;
    //save root node information for current unit
    TreeNode root_node[MAX_STATE];
    //the number of roots
    int num_root;
    int unit_state; 
}Unit;

typedef struct UnitList{
    Unit units[MAX_UNIT];
    int count_units;
}UnitList;

static inline void initUnit(Unit* unit)
{
    unit->count_start_states = -1;
    unit->count_unmatched_symbols = -1;
}

static inline void freeUnit(Unit* unit)
{

}

static inline void initUnitList(UnitList* ul)
{
    ul->count_units = -1;
}

static inline int getSizeUnitList(UnitList* ul)
{
    return ul->count_units+1;
}

//add query state information into unit
static inline void addStartingStates(Unit* unit, int* states, int num_states)
{
    for(int i = 0; i<num_states; i++)
    {
        unit->starting_states[i] = states[i];
    }
    unit->count_start_states = num_states-1;
}

//add unmatched ending symbol into unit
static inline void addUnmatchedSymbol(Unit* unit, int symbol)
{
    int index = (++unit->count_unmatched_symbols);
    unit->unmatched_symbols[index] = symbol;
    unit->unit_state = UNIT_UNMATCHED;
}

//add root node information for current unit
static inline void addRoots(Unit* unit, TreeNode* root, int num_root)
{
    int i;
    for(i = 0; i<num_root; i++)
        unit->root_node[i] = root[i];
    unit->num_root = num_root;
}

//add unit information into unit list
static inline void addUnit(UnitList* ul, Unit unit)
{
    int index = (++ul->count_units);
    ul->units[index] = unit;
}

#endif // !__UNIT_H__
