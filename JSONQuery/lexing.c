#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include "basic.h"
#include "lexing.h"

int jsl_next_token(Lexer* lexer)
{
    char* start = lexer->current_start;
    char* p = lexer->next_start;
    char* head = lexer->begin_stream;
    char* end = lexer->end_stream;
    int state = lexer->lex_state; //lex_state 
    int templen = 0;
    int token_len = 0;
    char* token_pointer = p; int tempcount;
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
                        lexer->lex_state = 0;
                        lexer->next_start = p+1;  
                        return LCB;
                    case '}':
                        token_len = p - start + 1;
                        token_pointer = start + token_len;
                        lexer->current_start = token_pointer;
                        lexer->lex_state = 0;
                        lexer->next_start = p+1;
                        return RCB;
                    case '[':
                        token_len = p - start + 1;
                        token_pointer = start + token_len;
                        lexer->current_start = token_pointer;
                        lexer->lex_state = 0;
                        lexer->next_start = p+1;
                        return LB;
                    case ']':
                        token_len = p - start + 1;
                        token_pointer = start + token_len;
                        lexer->current_start = token_pointer;
                        lexer->lex_state = 0;
                        lexer->next_start = p+1;
                        return RB;
                    case ',': 
                        token_len = p - start + 1;
                        token_pointer = start + token_len;
                        lexer->current_start = token_pointer;
                        lexer->lex_state = 0;
                        lexer->next_start = p+1;  
                        return COM;
                    case '"': //text or key field
                        token_len = p - start;
                        token_pointer = start + token_len;
                        start = token_pointer;
                        state = 1;  //printf("to 1\n");
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
                        char* sub1; 
                        sub1=substring(start , 1, tempcount);                
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
                            if(sub1!=NULL)
                            {
                                strcopy(sub1,lexer->content);
                                free(sub1);
                            }
                            token_len = p - start+1;
                            token_pointer = start + token_len;
                            lexer->current_start = token_pointer; 
                            p++;
                            lexer->lex_state = 0;
                            lexer->next_start = p+1; 
                            return KY;
                        }
                        else //primitive value
                        {
                            lexer->content[0]='"';
                            strcopy(sub1, lexer->content+1);
                            lexer->content[strlen(lexer->content)]='"'; 
                            lexer->content[strlen(lexer->content)+1]='\0';
                            if(sub1!=NULL) free(sub1);
                            lexer->lex_state = 0; 
                            lexer->next_start = p+1; 
                            return PRI;
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
                        char* sub1;
                        if(p[0]=='"'){
                            sub1=substring(start, 1 ,tempcount); 
                        }
                        else if(token_pointer[tempcount-1]==',') sub1=substring(token_pointer, 0, tempcount-1);
                        else sub1=substring(token_pointer, 0, tempcount);

                        if(sub1!=NULL)  //primitive value
                        {
                            strcopy(sub1,lexer->content);
                            free(sub1);
                        }
                        token_len = p - start + 1;
                        token_pointer = start + token_len;
                        lexer->current_start = token_pointer;
                        p--;
                        lexer->lex_state = 0;
                        lexer->next_start = p+1;
                        return PRI;
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
    return END;
}
