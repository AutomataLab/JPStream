#ifndef __FILE_PARTITION_H__
#define __FILE_PARTITION_H__

#include <stdio.h>

typedef struct PartitionInfo
{
    char** stream;
    int num_chunk;
    int* start_pos;
}PartitionInfo;

//partitioned intput stream into several chunks
PartitionInfo partitionFile(char* file_name, int num_core);

static inline void freeInputChunks(PartitionInfo pInfo)
{
    char** stream = pInfo.stream;
    int num_chunk = pInfo.num_chunk;
    int* start_pos = pInfo.start_pos;
    int i;
    for(i = 0; i<num_chunk; i++)
    {
        if(stream[i]!=NULL) free(stream[i]);
    }
    if(start_pos!=NULL) free(start_pos);
}

static inline void printChunk(char** stream, int num_core)
{
    printf("start printing paritioned chunks, total number of chunks is %d\n", num_core);
    for(int i = 0; i<num_core; i++)
    {
        printf("%dth chunk is %s\n", i, stream[i]); 
    }    
}
#endif // !__FILE_PARTITION_H__
