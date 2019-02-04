#ifndef __JSON_STREAM_H__
#define __JSON_STREAM_H__

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "basic.h"
#include "lexer.h"

#define MAX_CHUNK 100

typedef struct JsonStream{
    char* input_stream[MAX_CHUNK];
    int chunks_num;   //number of splitted chunks, for sequential version, this is 1
}JSONStream;


static inline void jps_JSONStreamCtor(JSONStream* stream, char* file_name, int thread_num)
{
    if(thread_num==1)
    {
        FILE *fp; 
        int size;
        fp = fopen (file_name,"rb");
        if (fp==NULL) { return;}
        fseek (fp, 0, SEEK_END);
        size=ftell (fp);
        rewind(fp);
        stream->input_stream[0]=(char*)malloc((size+1)*sizeof(char));
        stream->chunks_num = fread (stream->input_stream[0],1,size,fp);
        stream->input_stream[0][size]='\0';
        fclose(fp);
        //printf("load %d\n", strlen(stream->input_stream[0]));
    }
}

static inline void jps_JSONStreamDtor(JSONStream* stream)
{
    int i;
    for(i=0; i<stream->chunks_num; i++)
    {
        if(stream->input_stream[i]!=NULL) free(stream->input_stream[i]);
    }
    free(stream);
}

static inline JSONStream* jps_createJSONStream(char* file_name, int thread_num) //thread_num: number of threads
{
    JSONStream* stream = (JSONStream*)malloc(sizeof(JSONStream));
    jps_JSONStreamCtor(stream, file_name, thread_num);
    return stream;
}

static inline void jps_freeJSONStream(JSONStream* stream)
{  
    jps_JSONStreamDtor(stream);
    free(stream);
}

#endif // !JSON_STREAM_H__
