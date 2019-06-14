#include "file_partition.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "utility.h"

#define MAX_EXTENSION 1000

PartitionInfo partitionFile(char* file_name, int num_core)
{ 
    PartitionInfo pInfo;
    pInfo.num_chunk = 0;
    char** stream=NULL;
    int* start_pos = NULL;
    FILE *fp; 
    long file_size, chunk_size;
    fp = fopen(file_name,"rb");
    if (fp==NULL) { return pInfo;}
    fseek (fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);     

    chunk_size = (file_size/num_core)+1;
    stream = (char**)malloc(num_core*sizeof(char*));
    start_pos = (int*)malloc(num_core*sizeof(int));

    int i;
    for(i = 0; i<num_core; i++)
        stream[i] = NULL;
    long sum_size = 0;   //the number of bytes that have been processed
    char ch = -1; 
    for(i = 0; i<num_core-1; i++)
    {   
        start_pos[i] = sum_size;
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
        else fread (stream[i],1,chunk_size,fp);
        int add = 0;
        //look for the next complete token
        ch = fgetc(fp);
        while(1)
        {    
            if(ch==EOF) break;
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
        if((sum_size+chunk_size)>=file_size) {i++; break;}
    }
    pInfo.num_chunk = i;
    long remain_size = file_size-sum_size;
    if(remain_size>0)
    {
        start_pos[i] = sum_size;
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
    }
    pInfo.num_chunk = i+1;
    pInfo.stream = stream;
    pInfo.start_pos = start_pos;
    printf("file partitioner: finish splitting the input stream\n");
    return pInfo;
}
