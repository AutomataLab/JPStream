#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "file_operation.h"
#include "basic.h"
#include "global.h"


/*************************************************
Function: int split_file(char* file_name,int n);
Description: split a large file into several parts according to the number of threads for this program, while keeping the split JSON files into the memory
Input: file_name--the name for the xml file; n--the number of threads for this program
Return: the number of threads(start with 0); -1--can't open the JSON file
*************************************************/
int split_file(char* file_name,int n)
{
	FILE *fp;
	long i,j,k;
	long size, one_size;
	fp = fopen (file_name,"rb");
	if (fp==NULL) 
	{ 
	    return -1; 
	}
	fseek (fp, 0, SEEK_END);
    size=ftell (fp);
    rewind(fp); 
	one_size=(size/n)+1;
	n=n-1;
	char ch=-1;
	long temp_one_size=0;
	long s;
	
	for (i=0;i<n;i++)
	{
		s=0;
		buffFiles[i]=(char*)malloc((one_size+MAX_LINE*5)*sizeof(char));
		if(ch!=-1)
		{
		    buffFiles[i][0]=ch;
		    buffFiles[i][1]='\0';
		    char * buff=(char*)malloc((one_size)*sizeof(char));
		    k = fread (buff,1,one_size-1,fp);
		    buff[one_size-1]='\0'; 
		    buffFiles[i]=strcat(buffFiles[i],buff);
		    }
		    else
		    {
			k = fread (buffFiles[i],1,one_size,fp);
			}
		    /*skip the default size to look for the next open angle bracket*/
		    ch=fgetc(fp);
		    int tag=0;
            int t=one_size;
		    while(ch!='{'&&ch!='}'&&ch!=','&&ch!='	')
		    {
			    buffFiles[i][one_size+(s++)]=ch;
			    ch=fgetc(fp);
			    temp_one_size++;
			    if(ch==',')
			    {
				    int f;
				    for(f=s-1;f>=0;f--)
				    {
					    if(buffFiles[i][one_size+f]==':'||buffFiles[i][one_size+f]=='}'||buffFiles[i][one_size+f]==']')
					    {
						    tag=1;
							break;
						}
						else if(buffFiles[i][one_size+f]=='"'&&buffFiles[i][one_size+f-1]!='\\')
						{
							if(f==s-1) tag=1;
							else tag=0;
							break;
						}
					}
					if(tag==1) break;
					else 
					{
					    ch=fgetc(fp);
						continue;
					}
				}
			}

			if(ch==','||ch=='}')
			{
				buffFiles[i][one_size+(s++)]=ch;
			    ch=fgetc(fp);
			    temp_one_size++;
			}
			buffFiles[i][one_size+s]='\0'; 
	    }
	    j = size % one_size-temp_one_size;
	    if (j!=0) {
	        buffFiles[n]=(char*)malloc((j+1)*sizeof(char));
	        if(ch!=-1)
	        {
		        buffFiles[n][0]=ch;
		        buffFiles[n][1]='\0';
		        char * buff=(char*)malloc((j+1)*sizeof(char));
		        k = fread (buff,1,j,fp);
		        buff[j+1]='\0'; 
		        buffFiles[n]=strcat(buffFiles[n],buff);
		        buffFiles[n][j]='\0'; 
		    }
	        else
	        {
		        k = fread (buffFiles[n],1,j,fp);
		        buffFiles[n][j]='\0';  
		    }
	    }  
	   fclose(fp);
	    return n;
	}

/*************************************************
Function: int load_file(char* file_name);
Description: load the JSON file into memory(only used for sequential version)
Input: file_name--the name for the xml file
Return: 0--load successful; -1--can't open the XML file
*************************************************/
int load_file(char* file_name)
{
    FILE *fp;
    int i,j,k,n;
    int size;
    fp = fopen (file_name,"rb");
    if (fp==NULL) { return -1;}
    fseek (fp, 0, SEEK_END);
    size=ftell (fp);
    rewind(fp); 
    buffFiles[0]=(char*)malloc((size+1)*sizeof(char));
    k = fread (buffFiles[0],1,size,fp); 
    buffFiles[0][size]='\0';
    fclose(fp);
    return 0;
}

/*************************************************
Function: int write_file(char* file_name)
Description: write output array into file
Input: file_name--output file name
Return: 0--load successful; -1--can't open the target file
*************************************************/
int write_file(char* file_name)
{
    FILE *fp;
    int i,j,k,n;
    int size;
    remove(file_name);
    fp = fopen (file_name,"a");
    if (fp==NULL) { return -1;}

    for(i=0;i<=outputs[0].top_output;i++)
    {
        fprintf(fp,"%s\n",outputs[0].output[i]);
    }
    fclose(fp);
    return 0;
}

/*************************************************
Function: int write_file_with_predicates(char* file_name, int thread_numbers)
Description: write output array into file (for predicates)
Input: file_name--output file name
Return: 0--load successful; -1--can't open the target file
*************************************************/
int write_file_with_predicates(char* file_name, int thread_numbers)
{
    FILE *fp;
    int i,j,k,n;
    int size;
    remove(file_name);
    fp = fopen (file_name,"a");
    if (fp==NULL) { return -1;}
    /*for(i=0;i<=thread_numbers;i++)
    { 
    	for(j=0;j<=outputs[2*i+2].top_output;j++)
        {
            fprintf(fp,"%s\n",outputs[2*i+2].output[j]);
        }
	}*/
    for(i=0;i<=outputs[2].top_output;i++)
    {
        fprintf(fp,"%s\n",outputs[2].output[i]);
    }
    
    fclose(fp);
    return 0;
}
