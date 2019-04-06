#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utility.h"

/*************************************************
Function: void strcopy(char* src,char* dest)
Description: similar as strcpy
*************************************************/
void strcopy(char* src,char* dest)
{
	int length=strlen(src);
	int i;
	for(i=0;i<=length;i++)
	{
		dest[i]=src[i];
	}
	dest[i]='\0';
}

/**************************************************************************
Function: char* substring(char *pText, int begin, int end);
Description: print the substring of the original string
Input: pText--the original string; begin--start position; end--end position;
Return: the final string
***************************************************************************/
char* substring(char *pText, int begin, int end)
{
    int i,j;
    char * temp=pText;
    temp = ltrim(pText);
    char* temp1=NULL; 
    temp1=(char*)malloc((end-begin+1)*sizeof(char));
    for (j = 0,i = begin; i < end; i++,j++)
    {
        temp1[j]=temp[i];
    }
    temp1[j]='\0';
    return temp1;
}

/***************************************************************************************************
Function: char* substring_in_place(char* desc, char *pText, int begin, int end);
Description: save the substring of pText into a destination string
Input: desc -- the destination; pText--the original string; begin--start position; end--end position;
****************************************************************************************************/
void substring_in_place(char* desc, char *pText, int begin, int end)
{
    int i,j;
    char * temp=pText;
    temp = ltrim(pText);
    for (j = 0,i = begin; i < end; i++,j++)
    {
        desc[j]=temp[i];
    }
    desc[j]='\0'; 
}

/******************************************************************************
Function: char* allocate_and_copy(char *pText);
Description: allocate new memory spaces and copy the content of original string
Input: pText--the original string; 
Return: the new string
******************************************************************************/
char* allocate_and_copy(char* pText)
{
    char* new = (char*)malloc((strlen(pText)+2)*sizeof(char)); 
    strcopy(pText, new);
    return new;
}

/*************************************************
Function: char * ltrim(char *s);
Description: remove the left blankets of a string
Input: s--the original string; 
Return: the final substring
*************************************************/
char * ltrim(char *s)
{
     char *temp;
     temp = s;
     while((*temp == ' ')&&temp){*temp++;}
     return temp;
}

/*************************************************
Function: int left_null_count(char *s);
Description: calculate the number of blanket for each string
Input: s--the original string; 
Return: the number of blanket for each string
*************************************************/
int left_null_count(char *s)  
{
	 int count=0;
     char *temp;
     temp = s;
     while((*temp == ' ')&&temp){*temp++; count++;}
     return count;
}
