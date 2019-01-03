/**
 * This file is the runtime of dragontooth for C language.
 * It's a header-only library and also support C++.
 */

#ifndef __DRAGONTOOTH_H__
#define __DRAGONTOOTH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>



/* some special definition */

/* default configuration */

#if !defined(DTL_NO_LINENO) && !defined(DTL_LINENO)
#define DTL_NO_LINENO
#endif

#if !defined(DTL_NO_EQUIVALENCE_CLASS) && !defined(DTL_EQUIVALENCE_CLASS)
#define DTL_EQUIVALENCE_CLASS
#endif

#if !defined(DTL_NO_CUSTOM_ACTION) && !defined(DTL_CUSTOM_ACTION)
#define DTL_NO_CUSTOM_ACTION
#endif

#if !defined(DTP_NO_CUSTOM_ACTION) && !defined(DTP_CUSTOM_ACTION)
#define DTP_NO_CUSTOM_ACTION
#endif

#if !defined(DTP_NO_UNION_TYPE) && !defined(DTP_UNION_TYPE)
#define DTP_NO_UNION_TYPE
#endif

#if !defined(DTL_NO_ID_NAME) && !defined(DTL_ID_NAME)
#define DTL_ID_NAME
#endif

#if !defined(DT_NO_IMPL) && !defined(DT_IMPL)
#define DT_NO_IMPL
#endif

#define DTL_BUFFER_SIZE 100

#define DTL_EOF 0
#define DTL_ERROR_STATE 0

struct DTLexerState;
typedef uint32_t (*dtl_ActionCallback)(struct DTLexerState* ls,
                                       uint32_t action);

/* lexer custom action declaration */


struct DTParserState;
typedef uint32_t (*dtp_ActionCallback)(struct DTParserState* ls,
                                       uint32_t action);

/* parser custom action declaration */


#ifdef DTP_UNION_TYPE

/* union type definition */

#endif

/* table declaration */

///////////////////////////////////////////////////
/// Dragontooth Parser Stack
///////////////////////////////////////////////////

typedef struct DTStack {
    size_t size;
    size_t max_size;
    size_t* data;
} DTStack;

static inline void dts_StackCtor(DTStack* s) {
    s->size = 0;
    s->max_size = 10;
    s->data = (size_t*)malloc(s->max_size * sizeof(size_t));
}

static inline void dts_StackDtor(DTStack* s) {
    free(s->data);   
}

static inline DTStack* dts_CreateStack() {
    DTStack* stack = (DTStack*) malloc(sizeof(DTStack));
    dts_StackCtor(stack);
    return stack;
}

static inline void dts_FreeStack(DTStack* s) {
    dts_StackDtor(s);
    free(s);
}

static inline size_t dts_Back(DTStack* s) { return s->data[s->size - 1]; }

static inline void dts_Push(DTStack* s, size_t data) {
    if (s->size == s->max_size) {
        s->max_size *= 2;
        size_t* newdata = (size_t*)realloc(s->data, s->max_size * sizeof(size_t));
        if (newdata) s->data = newdata;
        // TODO: Be careful with the memory realloc failed
    }
    s->data[s->size++] = data;
}

static inline size_t dts_Pop(DTStack* s) {
    if (s->size > 0) s->size--;
    if (s->size == 0) return 0;
    return s->data[s->size - 1];
}

static inline size_t dts_MultiPop(DTStack* s, int n) {
    if (s->size >= n) s->size -= n;
    if (s->size == 0) return 0;
    return s->data[s->size - 1];
}

/**
 * @brief Mini push method can directly add a integer without checking the boundary
 * ONLY use it when you are sure there are some space in the array
 */
static inline void dts_mPush(DTStack* s, size_t data) {
    s->data[s->size++] = data;
}

/**
 * @brief Realloc function can resize the array
 */
static inline void dts_mRealloc(DTStack* s, size_t size) {
    if (size <= s->max_size) return;
    s->max_size = size;
    size_t* newdata =
        (size_t*)realloc(s->data, s->max_size * sizeof(size_t));
    if (newdata) s->data = newdata;
    // TODO: Be careful with the memory realloc failed
}



#ifdef DTP_UNION_TYPE

typedef struct DTStackUnion {
    size_t size;
    my_union_t* data;
    size_t max_size;
} DTStackUnion;

static inline void dts_UnionCtor(DTStackUnion* s) {
    s->data = (my_union_t*)malloc(10 * sizeof(my_union_t));
    s->max_size = 10;
    s->size = 0;
}

static inline void dts_UnionDtor(DTStackUnion* s) {
    free(s->data);
}

static inline my_union_t dts_BackUnion(DTStackUnion* s) { return s->data[s->size - 1]; }

static inline void dts_PushUnion(DTStackUnion* s, my_union_t data) {
    if (s->size == s->max_size) {
        s->data = (my_union_t*)realloc(s->data, s->max_size * 2);
        // TODO: Becareful with the memory realloc failed
        s->max_size *= 2;
    }
    s->data[s->size++] = data;
}

static inline my_union_t dts_GetUnion(DTStackUnion* s, int idx, int size) {
    return s->data[s->size - size + idx - 1];
}

static inline my_union_t dts_PopUnion(DTStackUnion* s) {
    if (s->size > 0) s->size--;
    if (s->size == 0) { my_union_t zero; zero.str = 0; return zero; }
    return s->data[s->size - 1];
}

static inline my_union_t dts_MultiPopUnion(DTStackUnion* s, int n) {
    if (s->size >= n) s->size -= n;
    if (s->size == 0) { my_union_t zero; zero.str = 0; return zero; }
    return s->data[s->size - 1];
}


#endif


///////////////////////////////////////////////////
/// Dragontooth Parallel Lexer DFA
///////////////////////////////////////////////////


typedef struct DTLP_DFA_CELL {
    int32_t next;
    int32_t* action;
} DTLP_DFA_CELL;

typedef struct DTLP_DFA {
    uint32_t row;
    uint32_t max_row;
    uint32_t input_size;
    DTLP_DFA_CELL* data;
} DTLP_DFA;


static inline void dtlp_jit_print(DTLP_DFA* dfa) {
    for (int i = 0; i < dfa->row; ++i) {
        printf("%2d: ", i);
        for (int j = 0; j < dfa->input_size; ++j) {
            printf("%d ", dfa->data[i*dfa->input_size+j].next);
        }
        printf("\n");
    }
}

static inline void dtlp_dfa_ctor(DTLP_DFA* dfa, int input_size, int max_row) {
    dfa->row = 2;
    dfa->max_row = max_row;
    dfa->input_size = input_size;
    dfa->data =
        (DTLP_DFA_CELL*)malloc(max_row * input_size * sizeof(DTLP_DFA_CELL));
    for (int i = 0; i < max_row * input_size; ++i) {
        dfa->data[i].next = -1;
        dfa->data[i].action = NULL;
    }
}

static inline void dtlp_dfa_dtor(DTLP_DFA* dfa) {
    free(dfa->data);
}

static inline DTLP_DFA* dtlp_dfa_create(int input_size, int max_row) {
    DTLP_DFA* dfa = (DTLP_DFA*)malloc(sizeof(DTLP_DFA));
    dtlp_dfa_ctor(dfa, input_size, max_row);
    return dfa;
}


///////////////////////////////////////////////////
/// Dragontooth Lexer
///////////////////////////////////////////////////

typedef struct DTLexerState {
    /* buffer is from outside input */
    const char* buffer;
    const char* head;

    uint32_t state;
    uint32_t begin_state;

#ifdef DTL_LINENO
    uint32_t line;
    uint32_t line_begin;
#endif

    FILE* input_file;
    char* input_buffer; /* this is when using file as the input */
    size_t buffer_size;
    uint8_t isFileFinished;
    uint8_t useFirstBuffer;
    /* four arrage defined automata table, you can change the data type when you
     * want to reduce the memory usage */
    const uint32_t* m_action;
    const int32_t* m_stop_state;
    const int32_t* m_default;
    const int32_t* m_base;
    const uint32_t* m_next;
    const uint32_t* m_check;
    const uint32_t* m_token;
    const uint32_t* m_ec;
    uint32_t bottom;  // this one is very important and must be set by user

#ifdef DTL_ID_NAME
    const char** m_id_name;
#endif
    dtl_ActionCallback action_callback;
    uint32_t state_num;   // how many states it has
    uint32_t input_size;  // how many kinds of input char
    DTStack* begin_stack;

    // This is the dfa in fast mode
    DTLP_DFA* fast_dfa; 
    DTStack* tokens; 
} DTLexerState;

/* buffer must have a zero as the tail */
static inline void dtl_SetBuffer(DTLexerState* ls, const char* buf) {
    ls->buffer = buf;
    ls->head = buf;
    ls->isFileFinished = 2;
}

static inline void dtl_readFile(DTLexerState* ls) {
    char* buf;
    size_t result;
    if (ls->useFirstBuffer) {
        buf = ls->input_buffer + ls->buffer_size;
        ls->head = buf;
    } else {
        buf = ls->input_buffer;
        ls->head = buf;
        char* end = ls->input_buffer + ls->buffer_size;
        *end = '\0';
    }
    result = fread(buf, 1, ls->buffer_size, ls->input_file);
    if (result == 0) ls->isFileFinished = 1;  // finished
    if (result < ls->buffer_size) {
        ls->isFileFinished = 2;  // will finished after this
        char* end = buf + result;
        *end = '\0';
    } else {
        ls->isFileFinished = 0;
        ls->useFirstBuffer = ~ls->useFirstBuffer;
    }
}

/* FILE must be readable */
static inline void dtl_SetFile(DTLexerState* ls, FILE* file) {
    char* buf = (char*)malloc(DTL_BUFFER_SIZE + 1);
    buf[DTL_BUFFER_SIZE] = 0;
    ls->input_file = file;
    ls->input_buffer = buf;
    ls->buffer_size = DTL_BUFFER_SIZE / 2;
    ls->useFirstBuffer = 0;
    dtl_SetBuffer(ls, buf);
    dtl_readFile(ls);
}

static inline void dtl_LexerCtor(DTLexerState* ls) {
#ifdef DTL_CUSTOM_ACTION
    ls->action_callback = NULL;
#endif
#ifdef DTL_LINENO
    ls->line = 0;
    ls->line_begin = 0;
#endif
    ls->begin_state = 1;
    ls->begin_stack = dts_CreateStack();
    ls->fast_dfa = NULL;
    dts_Push(ls->begin_stack, ls->begin_state);
}

static inline void dtl_LexerDtor(DTLexerState* ls) {
    dts_FreeStack(ls->begin_stack);
    if (ls->fast_dfa) {
        dtlp_dfa_dtor(ls->fast_dfa);
        free(ls->fast_dfa);
        ls->fast_dfa = NULL;
    }
}

static inline DTLexerState* dtl_Create() {
    DTLexerState* ls = (DTLexerState*)malloc(sizeof(DTLexerState));
    dtl_LexerCtor(ls);
    /* init the runtime when using codegen */
    return ls;
}

static inline void dtl_Free(DTLexerState* s) {
    dtl_LexerDtor(s);
    free(s);
} 

static inline DTLexerState* dtl_CreateWithBuffer(const char* buf) {
    DTLexerState* ls = dtl_Create();
    dtl_SetBuffer(ls, buf);
    return ls;
}

static inline DTLexerState* dtl_CreateWithFile(FILE* file) {
    DTLexerState* ls = dtl_Create();
    dtl_SetFile(ls, file);
    return ls;
}

static inline void dtl_SetActionCallback(DTLexerState* ls,
                                         dtl_ActionCallback cb) {
    ls->action_callback = cb;
}

static inline uint32_t dtl_nextStateImpl(DTLexerState* ls, uint32_t s,
                                         char ch) {
    int32_t k;
    if (s == 0 || ch == 0) return 0;
    k = ls->m_base[s] + (int32_t)ch;
    if ((k >= 0) && (k < ls->bottom) && (ls->m_check[k] == s))
        return ls->m_next[k];
    else {
        return -ls->m_default[s];
    }
}

static inline uint32_t dtl_nextState(DTLexerState* ls, uint32_t s, char ch) {
    int32_t k;
    if (s == 0 || ch == 0) return 0;
    k = ls->m_base[s] + (int32_t)ch;
    if ((k >= 0) && (k < ls->bottom) && (ls->m_check[k] == s))
        return ls->m_next[k];
    else {
        if (ls->m_default[s] > 0)
            return dtl_nextStateImpl(ls, ls->m_default[s], ch);
        return -ls->m_default[s];
    }
}

static inline int32_t dtl_isStopState(DTLexerState* ls, uint32_t s) {
    return ls->m_stop_state[s];
}

typedef struct DTLexerToken {
    bool action; /* if action is active, number of action is defined as same as
                    the status number */
    uint32_t line;      /* line of begin position */
    size_t line_begin;  /* postion in buffer, buffer[line_begin] is the begin of
                           this line */
    size_t size;        /* the size of this token */
    const char* data;   /* actual data pointer */
    const char* buffer; /* buffer pointer */
    uint8_t long_token; /* long token flag, if this is true, that means the
                           token is longer than the buffer, the dtl_Next will
                           still get this token at the next class. */
    char save_first;    /* this char is to save the next char after this token,
                           because we will use a \0 to put here */
    uint8_t save_flag;
    uint32_t rule;      /* this is the rule id that matches */
    uint32_t final_state;   /* get the final state after run dtl_Next */
} DTLexerToken;

/**
 * Please free the return buffer after use
 */
static inline char* dtl_SaveToken(DTLexerToken* token) {
    char* buffer = (char*)malloc(token->size + 1);
    int i, j;
    for (i = 0; i < token->size; ++i) {
        buffer[i] = token->data[i];
        if (buffer[i] == '\0') break;
    }
    if (i < token->size) {
        j = i;
        for (; i < token->size; ++i) {
            buffer[i] = token->buffer[i - j];
        }
    }
    buffer[token->size] = '\0';
    return buffer;
}

/**
 * This function will save the data without the " before and after the content
 */
static inline char* dtl_SaveString(DTLexerToken* token) {
    char* buffer = (char*)malloc(token->size - 1);
    int i, j;
    for (i = 0; i < token->size; ++i) {
        buffer[i] = token->data[i + 1];
        if (buffer[i] == '\0') break;
    }
    if (i < token->size) {
        j = i;
        for (; i < token->size; ++i) {
            buffer[i] = token->buffer[i - j];
        }
    }
    buffer[token->size - 1] = '\0';
    return buffer;
}

const uint32_t ac_return = 1 << (0 + 24), ac_push = 1 << (1 + 24),
               ac_begin = 1 << (2 + 24), ac_save_token = 1 << (3 + 24),
               ac_more = 1 << (4 + 24), ac_ignore = 1 << (5 + 24),
               ac_pop = 1 << (6 + 24), ac_common_action = 1 << (7 + 24);

static inline bool dtl_checkAction(uint32_t code, uint32_t action) {
    return (code & action) != 0;
}

static inline void dtl_ActionPrint(int32_t code) {
    if (dtl_checkAction(code, ac_return)) {
        int32_t k = (code >> 0) & 255;
        printf("action return(%d)\n", k);
    }
    if (dtl_checkAction(code, ac_push)) {
        int32_t k = (code >> 8) & 255;
        printf("action push(%d)\n", k);
    }
    if (dtl_checkAction(code, ac_begin)) {
        int32_t k = (code >> 16) & 255;
        printf("action begin(%d)\n", k);
    }
    if (dtl_checkAction(code, ac_save_token)) {
        printf("action save_token()\n");
    }
    if (dtl_checkAction(code, ac_more)) {
        printf("action more()\n");
    }
    if (dtl_checkAction(code, ac_ignore)) {
        printf("action ignore()\n");
    }
    if (dtl_checkAction(code, ac_pop)) {
        printf("action pop()\n");
    }
    if (dtl_checkAction(code, ac_common_action)) {
        printf("action common_action()\n");
    }

}

/**
 * dtl_ActionDecode will decode the action code from the compressed code
 * return 1 means the action will continue
 * return 0 is the end of this code
 */
static inline bool dtl_ActionDecode(DTLexerState* ls, uint32_t* begin_state,
                                    int32_t rule, uint32_t* state,
                                    DTLexerToken* token) {
    uint32_t code = ls->m_action[rule];
    if (dtl_checkAction(code, ac_return)) {
        // TODO: This action not implement
    }
    if (dtl_checkAction(code, ac_push)) {
        // this action will change the begin state
        int k = (code >> 8) & 255;
        dts_Push(ls->begin_stack, *begin_state);
        *begin_state = k;
    }
    if (dtl_checkAction(code, ac_begin)) {
        // this action will change the begin state
        int k = (code >> 16) & 255;
        *begin_state = k;
    }
    if (dtl_checkAction(code, ac_save_token)) {
        token->save_flag = 1;
    }
    if (dtl_checkAction(code, ac_common_action)) {
        token->action = true;
    }
    if (dtl_checkAction(code, ac_pop)) {
        *begin_state = dts_Pop(ls->begin_stack);
    }
    if (dtl_checkAction(code, ac_more)) {
        /** ac_more is just like ignore but it will not record the new postion
         * of token */
        *state = *begin_state;
        return true;
    }
    if (dtl_checkAction(code, ac_ignore)) {
        /* meet ignore token, ignore the data and move to the begin */
        *state = *begin_state;
        token->data = ls->head;
        return true;
    }
    return false;
}
static inline uint32_t dtl_getToken(DTLexerState* ls, int32_t rule_id) {
    return ls->m_token[rule_id];
}

#ifdef DTL_ID_NAME
static inline const char* dtl_getTokenName(DTLexerState* ls, int32_t rule_id) {
    return ls->m_id_name[rule_id];
}
#endif

static inline int dtl_check_stop(DTLexerState* ls, DTLexerToken* token, uint32_t* state) {
    int32_t action_token_id;
    
    int32_t p = dtl_isStopState(ls, *state);
    token->rule = p;
    if (p) {
        if (dtl_ActionDecode(ls, &(ls->begin_state), p, state,
                                token)) {
            return -2;
        } else {
            if (ls->head < token->data)
                token->size = ls->head + 2 * ls->buffer_size - token->data;
            else
                token->size = ls->head - token->data;
#ifdef DTL_CUSTOM_ACTION
            if (ls->action_callback && token->action) {
                action_token_id = ls->action_callback(ls, p);
                token->action = false;
                if (action_token_id != -1) return action_token_id;
            }
#endif
            return dtl_getToken(ls, p);
        }
    } else {
        /* error state */
        return -1;
    }
}

#ifdef DTL_EQUIVALENCE_CLASS
    #define dtl_eq_mapping(ls, h) (ls->m_ec[h])
#else
    #define dtl_eq_mapping(ls, h) (h)
#endif

/**
 * dtl_Next can get the next token id, token structure can not be changed during
 * the calculation
 */
static inline int32_t dtl_Next(DTLexerState* ls, DTLexerToken* token) {

    unsigned char h;
    uint32_t state, next_state;

    /* TODO: Long token support (a token which is longer than two buffers) */
    state = ls->begin_state;
    token->data = ls->head; /* now, it maybe recovered by other buffer */
    token->action = false;
    token->save_flag = 0; /* those flag will set after the action */
    token->buffer = ls->buffer;
    do {
        h = *(ls->head);

        // FIXME: There is a problem, if the token is not finished but reach the EOF, 
        //        it won't show error
        while (h != DTL_EOF) {
#ifdef DTL_LINENO
            if (h == '\n') {
                ls->line++;
                ls->line_begin = (ls->head - ls->buffer) + 1;
            }
#endif

            next_state = dtl_nextState(ls, state, dtl_eq_mapping(ls, h));

            // printf("%d : %d -> %d\n", state, ls->m_ec[ h ], next_state);
            if (next_state == DTL_ERROR_STATE) {
                int code = dtl_check_stop(ls, token, &state);
                if (code == -2) continue; else return code;
            }
            state = next_state;
            ls->head++;
            h = *(ls->head);
        }
        if (ls->isFileFinished == 0)
            dtl_readFile(ls);  // FIX ME: this logic is not right, readFile even
                               // has some data, will also set the file finished
        else if (ls->isFileFinished == 2) {
            ls->isFileFinished = 1;
            int code = dtl_check_stop(ls, token, &state);	
            if (code != -2) return code;
        }
    } while (ls->isFileFinished != 1);
    token->final_state = state;
    return DTL_EOF;
}

static inline void dtl_fastmode_init(DTLexerState* ls) {
    uint32_t state, next, begin_state;
    DTLexerToken token;
    DTLP_DFA* dfa = dtlp_dfa_create(ls->input_size, ls->state_num);
    dfa->row = dfa->max_row;
    for (uint32_t i = 0; i < ls->state_num; ++i)
        for (int j = 0; j < ls->input_size; ++j) {
            state = i;
            begin_state = 1;
            int* action = NULL;
        redo3:       
            next = dtl_nextState(ls, state, j);
            if (next == DTL_ERROR_STATE) {
                /* not move to a error state or stop state */
                int32_t p = dtl_isStopState(ls, state);
                if (p) {
                    if (!dtl_ActionDecode(ls, &begin_state, p, &next,
                                      &token)) {
                        action = (int*)calloc(1, sizeof(int));
                        action[0] = dtl_getToken(ls, p);
                    }
                    state = begin_state;
                    goto redo3;
                } else {
                    /* error state */
                }
            }
            DTLP_DFA_CELL cell = {(int)next, action};
            dfa->data[i * ls->input_size + j] = cell;
        }
    ls->fast_dfa = dfa;
    ls->tokens = dts_CreateStack();

    // dtlp_jit_print(dfa);
}

static inline DTLP_DFA_CELL dtpl_dfa_nextState(DTLP_DFA* dfa, uint32_t state, uint32_t g) {
    return dfa->data[dfa->input_size * state + g];
}

static inline uint32_t dtpl_dfa_transition(DTLexerState* ls, DTLP_DFA* dfa, uint32_t state, uint32_t g) {
    DTLP_DFA_CELL cell = dtpl_dfa_nextState(dfa, state, g);
    int32_t* action = cell.action;
    
    if (action != NULL) {
        dts_Push(ls->tokens, action[0]);
    }
    return cell.next;
}

static inline void dtl_fastmode(DTLexerState* ls, DTLexerToken* token) {
    uint32_t g;
    unsigned char h;
    uint32_t state = 1;
    DTLP_DFA* dfa = ls->fast_dfa;
    do {
        h = *(ls->head);
        while (h != DTL_EOF) {
            g = dtl_eq_mapping(ls, h);
            DTLP_DFA_CELL cell = dfa->data[dfa->input_size * state + g];
            int32_t* action = cell.action;
            if (cell.next == DTL_ERROR_STATE) { 
                printf("Error Detect\n");
                return; 
            }
            if (action != NULL) {
                dts_Push(ls->tokens, action[0]);
            }
            state = cell.next;
            ls->head++;
            h = *(ls->head);
        }
        if (ls->isFileFinished == 0)
            dtl_readFile(ls);
        else if (ls->isFileFinished == 2) {
            ls->isFileFinished = 1;
            g = dtl_eq_mapping(ls, h);
            state = dtpl_dfa_transition(ls, dfa, state, g);
        }
    } while (ls->isFileFinished != 1);
    token->final_state = state;
    dts_Push(ls->tokens, DTL_EOF);
}

#if defined(DT_IMPL) && defined(DTL_CUSTOM_ACTION)
/* lexer custom action function */

#endif

#if defined(DT_IMPL) && defined(DTP_CUSTOM_ACTION)
/* parser custom action function */

#endif

#ifdef DT_IMPL
/* implementation */

#endif

#ifdef __cplusplus
}
#endif

#endif /* __DRAGONTOOTH_H__ */