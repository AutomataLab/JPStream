#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/file.h>
#include <unistd.h>
#include <sched.h>
#include "basic.h"
//#include "global.h"
#include "lexer.h"

/************************************************************************************************************************************************
Function: int lexer(xml_Text *pText, xml_Token *pToken, token_info* tInfo, char* start_text);
Description: getting the next token from JSON data
Input: pText-the content of the json file; pToken-the type of the current json element; tInfo saves current running information, so that it can 
return the next following tokens when it is called again; char* start_text saves the contents in a string text
Return: 0--success -1--error 
*************************************************************************************************************************************************/
int lexer(xml_Text *pText, xml_Token *pToken, token_info* tInfo, char* start_text)
{
    char* start = tInfo->start;
    char* start1 = tInfo->head;
    int special_tag=0;
    int first_end_tag=0;

    char* p = tInfo->current;
    char* end = tInfo->end;
    int state = *tInfo->lex_state; //lex_state 
    int templen = 0;
    int i,j,a;

    int flags=0;
    pToken->text.p = p;
    int flag_attribute=0;
    int new_tag=0;
    char *start2=p;
    int segflag = 0;

    for (; p < end; p++)
    {   
        switch(state)
        {
            case 0:
               switch(*p)
               {
                    case '{':
                        pToken->text.len = p - start + 1;
                        pToken->text.p = start + pToken->text.len;
                        tInfo->start = pToken->text.p;
                        *tInfo->lex_state = 0;
                        tInfo->current = p+1;
                        return LCB;
                    case '}':
                        pToken->text.len = p - start + 1;
                        pToken->text.p = start + pToken->text.len;
                        tInfo->start = pToken->text.p;
                        *tInfo->lex_state = 0;
                        tInfo->current = p+1;
                        return RCB;
                    case '[':
                        pToken->text.len = p - start + 1;
                        pToken->text.p = start + pToken->text.len;
                        tInfo->start = pToken->text.p;
                        *tInfo->lex_state = 0;
                        tInfo->current = p+1;
                        return LB;
                    case ']':
                        pToken->text.len = p - start + 1;
                        pToken->text.p = start + pToken->text.len;
                        tInfo->start = pToken->text.p;
                        *tInfo->lex_state = 0;
                        tInfo->current = p+1;
                        return RB;
                    case ',': 
                        pToken->text.len = p - start + 1;
                        pToken->text.p = start + pToken->text.len;
                        tInfo->start = pToken->text.p;
                        *tInfo->lex_state = 0;
                        tInfo->current = p+1;
                        return COM;
                    case '"': //text or key field
                        pToken->text.len = p - start  ;
                        pToken->text.p = start + pToken->text.len;
                        start = pToken->text.p;
                        state = 1;
                        break;
                    case ' ':
                    case '       ':  //jump space
                        pToken->text.len = p - start + 1;
                        pToken->text.p = start + pToken->text.len;
                        start = pToken->text.p;
                        state = 0;
                        break;
                    default: //it might be a single primitive
                        if((*p>='0'&&*p<='9')||(*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')) state=2;
                        else
                        {
                            pToken->text.len = p - start + 1;
                            pToken->text.p = start + pToken->text.len;
                            start = pToken->text.p;
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
                        int tempcount = p-start;
                        templen = tempcount;
                        char* sub1;
                        sub1=substring(pToken->text.p , 1 , tempcount);
                        if(sub1!=NULL)
                        {
                            strcopy(sub1,start_text);
                            free(sub1);
                        }
                        pToken->text.len = p - start + 1;
                        pToken->text.p = start + pToken->text.len;
                        start = pToken->text.p;

                        while(*(p+1)==' '||*(p+1)=='        ') //ignore space
                        {
                            p++;
                            pToken->text.len = p - start + 1;
                            pToken->text.p = start + pToken->text.len;
                            start = pToken->text.p;
                        }
                        if(*(p+1)==':') //it is a key field
                        {
                            pToken->text.len = p - start+1 ;
                            pToken->text.p = start + pToken->text.len;
                            tInfo->start = pToken->text.p; p++;
                            *tInfo->lex_state = 0;
                            tInfo->current = p+1;
                            return KY;
                        }
                        else //primitive value
                        {
                            *tInfo->lex_state = 0; 
                            tInfo->current = p+1;
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
                    case '       ':
                   	{
                        int tempcount = p-start;
                        templen = tempcount;
                        char* sub1;
                        if(p[0]=='"')
                            sub1=substring(pToken->text.p , 1 , tempcount);
                        else sub1=substring(pToken->text.p, 0, tempcount);

                        if(sub1!=NULL)  //primitive value
                        {
                            strcopy(sub1,start_text);
                            free(sub1);
                        }
                        pToken->text.len = p - start + 1;
                        pToken->text.p = start + pToken->text.len;
                        tInfo->start = pToken->text.p; p--;
                        *tInfo->lex_state = 0;
                        tInfo->current = p+1;
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
    return -1;
}


