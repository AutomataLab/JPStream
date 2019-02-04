#ifndef __AUTOMATON_H__
#define __AUTOMATON_H__

#define MAX_STATE 500
#define MAX_TRANSITION 100
#define MAX_STRING 100

#include "dfa_builder.h"
#include "basic.h"

#define NOT_MATCH 0
#define OUTPUT_MATCH 1
#define PREDICATE_MATCH 2

typedef struct Transition{
    int start_state;
    char transition[MAX_STRING];
    char end_state;
}Transition;

typedef struct index_pair{
    int lower;
    int upper;
}index_pair;

typedef struct State{
    Transition* transition_table;
    int num_transitions;  
    index_pair range;  //range for array indexes
    int acc_type;      //type for accept states
}State;

typedef struct Automaton{
   State* states;
   int num_states;
}Automaton;

static inline void jpa_AutomatonCtor(Automaton* automaton, int num_state)
{
    automaton->states = (State*)malloc((num_state+1)*sizeof(State));
    automaton->num_states = num_state;
    int i;
    for(i=0; i<=num_state; i++)
    {
        automaton->states[i].num_transitions = -1;
    }
}

static inline void jpa_AutomatonDtor(Automaton* automaton)
{
    int i;
    for(i=0; i<=automaton->num_states; i++)
    {
        if(automaton->states[i].transition_table!=NULL) free(automaton->states[i].transition_table);
        /*for(j=0; j<=automaton->states[i].num_transition; j++)
        {
            if(automaton->states[i]->transition_table[j]!=NULL) free(automaton->states[i]->transition_table[j]);
        }
        if(automaton->states[i]!=NULL) free(automaton->states[i]);*/
    }
    free(automaton->states);
}

static inline Automaton* jpa_createAutomaton(int num_state)
{
    Automaton* automaton = (Automaton*)malloc(sizeof(Automaton));
    jpa_AutomatonCtor(automaton, num_state);
    return automaton;
}

static inline void jpa_freeAutomaton(Automaton* automaton)
{
    jpo_AutomatonDtor(automaton);
    free(automaton);
}

//load DFA table into query engine
static inline Automaton* jpa_loadDfaTable(JQ_DFA* dfa)
{
    //int i,j,k;
    int state,input;
    int num_state = jqd_getStatesNum(dfa);
    Automaton* automaton = jpa_createAutomaton(num_state);
    automaton->num_states = num_state;
    if(automaton==NULL) return NULL;
    for(state = 1; state < num_state; state++)
    {
        //automaton->states[state].range.low = 0;
        //automaton->states[state].range.high = 0;
        //++stateCount; ++machineCount;
        //stateMachine[2*stateCount-1].n_transitions = -1;
        //stateMachine[2*stateCount-1].isoutput = 0;
        //stateMachine[2*stateCount-1].low = 0;
        //stateMachine[2*stateCount-1].high = 0;
        int stop_state = jqd_getAcceptType(dfa, state); 
        automaton->states[state].acc_type = stop_state;  
        automaton->states[state].transition_table = (Transition*)malloc(100*sizeof(Transition));  //I'll change this
        for(input = 2; input < jqd_getInputsNum(dfa); input++)
        {
            int next_state = jqd_nextState(dfa, state, input);
            if(next_state>0)
            {
                //int stop_state =  jqd_getStopState(dfa, next_state); //needs remove
                //if(stop_state>0) stateMachine[2*stop_state-1].isoutput = 2;   //needs remove
                int index = (++automaton->states[state].num_transitions);  //printf("state %d stop state %d state %d transitions %d\n", state, jqd_getStopState(dfa, next_state),next_state, automaton->states[state].num_transitions);
                //automaton->states[state].transition_table[index] = (Transition)malloc(sizeof(Transition));
                automaton->states[state].transition_table[index].start_state = state;
                automaton->states[state].transition_table[index].end_state = next_state;
                const char* transition = jqd_getName(dfa, input);
                strcopy(transition, automaton->states[state].transition_table[index].transition);
                if(input==2 && next_state!=0)
                {
                    JQ_index_pair pair = jqd_getArrayIndex(dfa, state);
                    automaton->states[next_state].range.lower = pair.lower;
                    automaton->states[next_state].range.upper = pair.upper;
                }
            }
        }
        int last_state = jqd_nextState(dfa, state, 1); ///printf("last_state %d\n",last_state);
        if(last_state>0)
        {
            int last_index = (++automaton->states[state].num_transitions);
            automaton->states[state].transition_table[last_index].start_state = state;
            automaton->states[state].transition_table[last_index].end_state = last_state;
            strcopy("*", automaton->states[state].transition_table[last_index].transition); 
        }
    }
}

static inline int jpa_nextState(Automaton* automaton, int state, char* str)
{
    //printf("in\n");
    return 0;
    int k; //printf("%d\n", state);
    for(k=0; k<=automaton->states[state].num_transitions; k++)
    {
        if((strcmp(automaton->states[state].transition_table[k].transition,"*")==0)||(strcmp(automaton->states[state].transition_table[k].transition,str)==0))
        {
            //if((automaton->states[state].range.lower>prior_counter || automaton->states[state].range.lower < prior_counter))
                //continue;
            break;
        }
   }
   if(k<=automaton->states[state].num_transitions)
       return automaton->states[state].transition_table[k].end_state;
   else return -1;
}

static inline int jpa_getStatesNum(Automaton* automaton)
{   
    return automaton->num_states;
}

static inline int jpa_getAcceptType(Automaton* automaton, int state)
{  
    return 0;//automaton->states[state].acc_type;
}

#endif // !__AUTOMATON_H__
