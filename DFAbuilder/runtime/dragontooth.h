/**
 * This file is the runtime of dragontooth for C language.
 * It's a header-only library and also support C++.
 */

#ifndef __DRAGONTOOTH_H__
#define __DRAGONTOOTH_H__

/* some special definition */

#if !defined(DTL_NO_LINENO) && !defined(DTL_LINENO)
#define DTL_LINENO
#endif

#if !defined(DTL_NO_EQUIVALENCE_CLASS) && !defined(DTL_EQUIVALENCE_CLASS)
#define DTL_EQUIVALENCE_CLASS
#endif



#ifdef __cplusplus
extern "C" {
#endif

#include <malloc.h>
#include <stdint.h>
#include <stdio.h>

typedef uint32_t (*dtl_ActionCallback)(struct DTLexerState* ls,
                                       uint32_t action);
#define DTL_BUFFER_SIZE 32768

#define DTL_EOF 0
#define DTL_ERROR_STATE 0


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
    /* four arrage defined automata table, you can change the data type when you */
    /* want to reduce the memory usage */
    const uint32_t* m_action;
    const int32_t* m_token;
    const int32_t* m_default;
    const int32_t* m_base;
    const uint32_t* m_next;
    const uint32_t* m_check;
    uint32_t bottom;
#ifdef DTL_EQUIVALENCE_CLASS
    const uint32_t* m_ec;
#endif
    dtl_ActionCallback action_callback;
} DTLexerState;

/* buffer must have a zero as the tail */
inline void dtl_SetBuffer(DTLexerState* ls, const char* buf) {
    ls->buffer = buf;
    ls->head = buf;
    ls->isFileFinished = 1;
}

inline void dtl_readFile(DTLexerState* ls) {
    char *buf; size_t result;
    if (ls->useFirstBuffer) {
        buf = ls->input_buffer + ls->buffer_size;
        ls->head = buf;
    } else { 
        buf = ls->input_buffer;
        ls->head = buf;
    }
    result = fread( buf, 1, ls->buffer_size + 1, ls->input_file );
    if (result == 0) 
        ls->isFileFinished = 1;  // finished
    if (result < ls->buffer_size + 1) {
        ls->isFileFinished = 2;  // will finished after this 
    } else {
        ls->isFileFinished = 0;
        ls->useFirstBuffer = ~ ls->useFirstBuffer;
    }
}

/* FILE must be readable */
inline void dtl_SetFile(DTLexerState* ls, FILE* file) {
    char* buf = (char*)malloc(DTL_BUFFER_SIZE + 1);
    buf[DTL_BUFFER_SIZE] = 0;
    ls->input_file = file;
    ls->input_buffer = buf;
    ls->buffer_size = DTL_BUFFER_SIZE / 2;
    ls->useFirstBuffer = 0;
    dtl_SetBuffer(ls, buf);
    dtl_readFile(ls);
}

inline DTLexerState* dtl_Create() {
    DTLexerState* ls = (DTLexerState*)malloc(sizeof(DTLexerState));
    ls->action_callback = NULL;
    ls->begin_state = 1;    
#ifdef DTL_LINENO            
    ls->line = 0;
    ls->line_begin = 0;
#endif
    /* init the runtime when using codegen */
    return ls;
}

inline DTLexerState* dtl_CreateWithBuffer(const char* buf) {
    DTLexerState* ls = dtl_Create();
    dtl_SetBuffer(ls, buf);
    return ls;
}

inline DTLexerState* dtl_CreateWithFile(FILE* file) {
    DTLexerState* ls = dtl_Create();
    dtl_SetFile(ls, file);
    return ls;
}


inline void dtl_SetActionCallback(DTLexerState* ls, dtl_ActionCallback cb) {
    ls->action_callback = cb;
}


inline uint32_t dtl_nextStateImpl(DTLexerState* ls, uint32_t s, char ch) {
    int32_t k;
    if (s == 0 || ch == 0) return 0;
    k = ls->m_base[s] + (int32_t)ch;
    if ((k >= 0) && (k < ls->bottom) && (ls->m_check[k] == s))
        return ls->m_next[k];
    else {
        return - ls->m_default[s];
    }
}

inline uint32_t dtl_nextState(DTLexerState* ls, uint32_t s, char ch) {
    int32_t k;
    if (s == 0 || ch == 0) return 0;
    k = ls->m_base[s] + (int32_t)ch;
    if ((k >= 0) && (k < ls->bottom) && (ls->m_check[k] == s))
        return ls->m_next[k];
    else {
        if (ls->m_default[s] > 0) return dtl_nextStateImpl(ls, ls->m_default[s], ch);
        return - ls->m_default[s];
    }
}

inline int32_t dtl_isStopState(DTLexerState* ls, uint32_t s) {
    return ls->m_token[s];
}

typedef struct DTLexerToken {
    uint32_t action;     /* action number */
    uint32_t line;       /* line of begin position */
    size_t line_begin;   /* postion in buffer, buffer[line_begin] is the begin of this line */
    size_t size;         /* the size of this token */
    const char* data;    /* actual data pointer */
    const char* buffer;  /* buffer pointer */
    uint8_t long_token;  /* long token flag, if this is true, that means the token is longer than
                            the buffer, the dtl_Next will still get this token at the next class. */
    char save_first;     /* this char is to save the next char after this token, because we will use a \0 to put here */
} DTLexerToken;

/**
 * Please free the return buffer after use
 */
inline char* dtl_SaveToken(DTLexerToken* token) {
    char* buffer = (char*) malloc(token->size+1);
    int i, j;
    for (i = 0; i < token->size; ++i) {
        buffer[i] = token->data[i];
        if (buffer[i] == '\0') break;
    }
    if (i < token->size) {
        j = i;
        for (; i < token->size; ++i) {
            buffer[i] = token->buffer[i-j];
        }
    }
    buffer[token->size] = '\0';
    return buffer;
}

/**
 * dtl_Next can get the next token id, token structure can not be changed during
 * the calculation
 */
inline int32_t dtl_Next(DTLexerState* ls, DTLexerToken* token) {
    int32_t token_id, action_token_id;
    unsigned char h; uint32_t state, next_state;

    /* TODO: Long token support (a token which is longer than two buffers) */
    state = ls->begin_state;
    token->data = ls->head; /* now, it maybe recovered by other buffer */
    token->buffer = ls->buffer;
    do {
        h = *(ls->head);

        while(h != DTL_EOF) {
#ifdef DTL_LINENO            
            if (h == '\n') {
                ls->line++;
                ls->line_begin = (ls->head - ls->buffer) + 1;
            }
#endif
#ifdef DTL_EQUIVALENCE_CLASS
            next_state = dtl_nextState(ls, state, ls->m_ec[ h ]);
#else
            next_state = dtl_nextState(ls, state, h);
#endif
            if (next_state == DTL_ERROR_STATE) {
                /* not move to a error state or stop state */
                int32_t p = dtl_isStopState(ls, state);
                if (p) {
                    if (p == 9) {
                        /* meet ignore token, ignore the data and move to the begin */
                        state = ls->begin_state;
                        token->data = ls->head;
                        continue;
                    } else {
                        token->size = ls->head - token->data;
                        // token->action = ls->m_action[state]; // TODO:Action
                        return p;
                    }
                } else {
                    /* error state */
                    return -1;
                }
            }

            state = next_state;
            ls->head++;
            h = *(ls->head);
        }
        if (ls->isFileFinished == 0)
            dtl_readFile(ls); // FIX ME: this logic is not right, readFile even has some data, will also set the file finished
        else if (ls->isFileFinished == 2) 
            ls->isFileFinished = 1;
    } while (ls->isFileFinished != 1);

    if (ls->action_callback && token->action) {
        action_token_id = ls->action_callback(ls, token->action);
        if (action_token_id != -1) return action_token_id;
    }
    return DTL_EOF;
}

#ifdef __cplusplus
}
#endif

#endif  /* __DRAGONTOOTH_H__ */