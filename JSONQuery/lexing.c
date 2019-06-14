#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include "utility.h"
#include "lexing.h"

#define MAX_PRIMITIVE 40000

Token nextToken(Lexer* lexer)
{
    char* start = lexer->current_pointer; 
    char* p = lexer->next_start;
    char* head = lexer->begin_stream;
    char* end = lexer->end_stream;
    int state = lexer->lex_state; //lex_state 
    int templen = 0;
    int token_len = 0;
    char* token_pointer = p; 
    int tempcount;
    Token token;
    char sub1[MAX_PRIMITIVE];
    for (; p < end; p++)
    {   
        switch(state)
        {
            case 0:
               switch(*p)
               {
                    case '{':
                        token_len = p - start + 1;
                        token_pointer = start + token_len;
                        lexer->current_start = token_pointer;
                        lexer->current_pointer = lexer->current_start;
                        lexer->lex_state = 0;
                        lexer->next_start = p+1; 
                        token.token_type = LCB; 
                        return token;
                    case '}':
                        token_len = p - start + 1;
                        token_pointer = start + token_len;
                        lexer->current_start = token_pointer;
                        lexer->current_pointer = lexer->current_start;
                        lexer->lex_state = 0;
                        lexer->next_start = p+1;
                        token.token_type = RCB;
                        return token;
                    case '[':
                        token_len = p - start + 1;
                        token_pointer = start + token_len;
                        lexer->current_start = token_pointer;
                        lexer->current_pointer = lexer->current_start;
                        lexer->lex_state = 0;
                        lexer->next_start = p+1;
                        token.token_type = LB;
                        return token;
                    case ']':
                        token_len = p - start + 1;
                        token_pointer = start + token_len;
                        lexer->current_start = token_pointer;
                        lexer->current_pointer = lexer->current_start;
                        lexer->lex_state = 0;
                        lexer->next_start = p+1;
                        token.token_type = RB;
                        return token;
                    case ',': 
                        token_len = p - start + 1;
                        token_pointer = start + token_len;
                        lexer->current_start = token_pointer;
                        lexer->current_pointer = lexer->current_start;
                        lexer->lex_state = 0;
                        lexer->next_start = p+1;  
                        token.token_type = COM;
                        return token;
                    case '"': //text or key field
                        token_len = p - start;
                        token_pointer = start + token_len;
                        start = token_pointer;
                        state = 1;  
                        break;
                    case ' ':
                    case '\t':  //jump space
                        token_len = p - start + 1;
                        token_pointer = start + token_len;
                        start = token_pointer;
                        state = 0;
                        break;
                    default: //it might be a single primitive
                        if((*p>='0'&&*p<='9')||(*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')) state=2;
                        else
                        {
                            token_len = p - start + 1;
                            token_pointer = start + token_len;
                            start = token_pointer;
                            state = 0;
                        }
                       break;
               }
            break;
            case 1:
               switch(*p)
               {
                   case '"':
                       //check backslashes
                        if(*(p-1)=='\\')
                        {
                            char* pointer = p-2;
                            int count = 1;
                            while(*pointer=='\\'){
                                count++;
                                --pointer;
                            }
                            if(count%2==1) {state = 1; break;}  //it is not a string
                        }
                        //string is found 
                        tempcount = p-start;
                        templen = tempcount; 
                        substring_in_place(sub1, start , 1, tempcount);     
                        int s_content = start + 1 -lexer->begin_stream;
                        int e_content = s_content + tempcount - 1;
                        token_len = p - start + 1;
                        token_pointer = start + token_len;
                        start = token_pointer;

                        while(*(p+1)==' '||*(p+1)=='\t') //ignore space
                        {
                            p++;
                            token_len = p - start + 1;
                            token_pointer = start + token_len;
                            start = token_pointer;
                        } 
                        if(*(p+1)==':') //it is a key field
                        {
                            strcopy(sub1,lexer->content);
                            token_len = p - start+1;
                            token_pointer = start + token_len;
                            lexer->current_start = lexer->current_pointer+1; 
                            lexer->current_pointer = token_pointer;
                            p++;
                            lexer->lex_state = 0;
                            lexer->next_start = p+1; 
                            token.token_type = KY;
                            token.content = lexer->content;
                            return token;
                        }
                        else //primitive value
                        {
                            lexer->current_start = lexer->current_pointer+1; 
                            lexer->lex_state = 0; 
                            lexer->next_start = p+1; 
                            lexer->start_content = s_content-1;
                            lexer->end_content = e_content+1;
                            token.token_type = PRI;
                            return token;
                        }
                        
                        state = 0;
                        break;
                    default:
                           state = 1;
                           break;
               }

            break;
            case 2:
                switch(*p)
                {
                    case '}':
                    case ']':
                    case ',':
                    case ' ':
                    case '\t':
                   	{
                        int tempcount = p-start;
                        templen = tempcount; 
                        int s_content = start + 1 -lexer->begin_stream;
                        int e_content = s_content + tempcount - 1;
                        if(p[0]=='"')
                        {
                            s_content = start + 1 -lexer->begin_stream;
                            e_content = s_content + tempcount - 1;
                        }
                        else if(token_pointer[tempcount-1]==',') 
                        {
                            s_content = token_pointer - lexer->begin_stream;
                            e_content = s_content + tempcount - 1;
                        }
                        else{
                            s_content = token_pointer - lexer->begin_stream;
                            e_content = s_content + tempcount;
                        } 
                        token_len = p - start + 1;
                        token_pointer = start + token_len;
                        lexer->current_pointer = token_pointer;
                        lexer->current_start = lexer->current_pointer+1; 
                        p--;
                        lexer->lex_state = 0;
                        lexer->next_start = p+1;
                        lexer->start_content = s_content;
                        lexer->end_content = e_content;
                        token.token_type = PRI;
                        return token;
                    }
                    default:
                        state = 2;
                        break;
                }
                break;

            default:
                state = -1;
                break;
        }
        
    }
    token.token_type = END;
    return token;
}
