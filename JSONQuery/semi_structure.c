#include <string.h>
#include <stdio.h>
#include "semi_structure.h"
#include "basic.h"

/*************************************************
Function: int xml_initText(xml_Text *pText, char *s);
Description: initiate a xml_Text for a string loading from original file
Input: pText--the xml_Text element waiting to be initialized; s--the semi-structure input;
Output: pText--the initialized xml_Text
Return: 0--success
*************************************************/
int xml_initText(xml_Text *pText, char *s)
{
///	printf("1\n");
    pText->p = s;
   /// printf("2 %d\n",strlen(s));
    pText->len = strlen(s);
    ///printf("3\n");
    return 0;
}

/*************************************************
Function: xml_initToken(xml_Token *pToken, xml_Text *pText);
Description: initiate a xml_Token for a initialized xml_Text
Input: pToken--the xml_Token element waiting to be initialized; pText--input xml_Text;
Output: pToken--the initialized xml_Token
Return: 0--success
*************************************************/
int xml_initToken(xml_Token *pToken, xml_Text *pText)
{
    pToken->text.p = pText->p;
    pToken->text.len = 0;
    pToken->type = xml_tt_U;
    return 0;
}

/*************************************************
Function: int xml_print(xml_Text *pText, int begin, int end);
Description: print the substring of a xml_Text
Input: pText--input xml_Text; begin--start position; end--end position;
Output: the substring of a xml_Text
Return: 0--success
*************************************************/
int xml_print(xml_Text *pText, int begin, int end)
{
    int i;
    char * temp=pText->p;
    temp = ltrim(pText->p);
    int j=0;
    for (i = begin; i < end; i++)
    {
        putchar(temp[i]);
    }
    return 0;
}

