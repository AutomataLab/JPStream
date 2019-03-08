#ifndef __FILE_PARTITION_H__
#define __FILE_PARTITION_H__

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "utility.h"

#define MAX_EXTENSION 1000

typedef struct PartitionInfo
{
    char** stream;
    int num_chunk;
}PartitionInfo;

//return partitioned chunks
char** partitionFile(char* file_name, int num_core)
{ 
    PartitionInfo pInfo;
    pInfo.num_chunk = 0;
    char** stream=NULL;
    FILE *fp; 
    long file_size, chunk_size;
    fp = fopen(file_name,"rb");
    if (fp==NULL) { return NULL;}
    fseek (fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);     

    chunk_size = (file_size/num_core)+1;
    stream = (char**)malloc(num_core*sizeof(char*));
    long sum_size = 0;   //the number of bytes that have been processed
    int i;
    char ch = -1; 
    for(i = 0; i<num_core-1; i++)
    {
        //load input stream with fixed sizes
        stream[i] = (char*)malloc((chunk_size+MAX_EXTENSION)*sizeof(char));
        if(ch!=-1)
        {
            stream[i][0]=ch;
            stream[i][1]='\0';
            char * buff=(char*)malloc((chunk_size)*sizeof(char));
            fread (buff,1,chunk_size-1,fp);
            buff[chunk_size-1]='\0'; 
            stream[i]=strcat(stream[i],buff);
        }
        else fread(stream[i],1,chunk_size,fp);
        int add = 0;
        //look for the next complete token
        ch = fgetc(fp);
       
        while(1)
        {
            if(ch=='"')
            {
                char prev = stream[i][chunk_size+add-1];
                int t = 1;
                while(prev==' '||prev=='\t'||prev=='\n'||prev==13)
                {
                    prev = stream[i][chunk_size+add-t]; 
                    t++;
                }
                if(prev==','||prev=='{'||prev=='[')
                {
                    break;
                }
            }
            stream[i][chunk_size+add] = ch; 
            add = add+1;
            ch = fgetc(fp);
        }
        stream[i][chunk_size+add] = '\0';
        sum_size += (chunk_size+add);
        if((sum_size+chunk_size)>file_size) {i++; break;}
    }
    long remain_size = file_size-sum_size;
    stream[i] = (char*)malloc((remain_size)*sizeof(char));
    if(ch!=-1)
    {
        stream[i][0]=ch;
        stream[i][1]='\0';
        char* buff=(char*)malloc(remain_size*sizeof(char));
        fread (buff,1,remain_size-1,fp);
        buff[remain_size-1]='\0';
        stream[i]=strcat(stream[i],buff);
        stream[i][remain_size] = '\0'; 
    }
    else{
        fread (stream[i],1,remain_size,fp);
        stream[i][remain_size] = '\0';
    }
    pInfo.num_chunk = i+1;
    pInfo.stream = stream;
    return stream;
}


void printChunk(char** stream, int num_core)
{
    printf("begin printing results %d\n", num_core);
    for(int i = 0; i<num_core; i++)
    {
        printf("\n%dth chunk is start: %s end: %s\n", i, substring(stream[i],0,100), substring(stream[i], strlen(stream[i])-100, strlen(stream[i])));
    }
    for(int i = 0; 1==2&&i<num_core; i++)
    {
        printf("\n%dth chunk is start: %s\n", i, stream[i]);
    }
}
#endif // !__FILE_PARTITION_H__
