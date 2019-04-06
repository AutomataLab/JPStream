#ifndef __UTILITY_H__
#define __UTILITY_H__

void strcopy(char* src,char* dest);
char* substring(char *pText, int begin, int end);
char* allocate_and_copy(char* pText);
char* ltrim(char *s); //reduct blank from left
int left_null_count(char *s);  //calculate the number of blanket for each string

#endif // !__UTILITY_H__
