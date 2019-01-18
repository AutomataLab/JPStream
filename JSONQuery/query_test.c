#include "query.h"
#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include "input.h"

void Test0()
{
    initialization();
    get_parameters_from_config("config");
    debug_flag = 2;
    automata();
}

void Test1()
{
    initialization();
    file_name = "twitter_store1.txt"; 
    jsonPath = "$.root[*].quoted_status.*.media[?(@.url)&&(@.media_url)].sizes.large.w";
    jsonPath = "$.root[*].entities.hashtags[*].text"; 
    num_threads = 16;
    pversion = 0;
    warmup_flag = 1;
    debug_flag = 1;
    percentage = 1;
    xmlPath=(char*)malloc(2000*sizeof(char)); 
    int ret = convertJSONPath(jsonPath, xmlPath);
    if(ret == 0)
    {
        printf("Input JSONPath query in config is not correct, please open the file and check it again!\n");
        return;
    }
    automata();
    ret = execute_query();
    if(ret==0)
    {
        printf("query execution exception, please check your input\n");
        return;
    }
    writing_results();
    if(debug_flag==1) print_debug_info(); 
}

void Test2()
{
    initialization();
    file_name = "twitter_store1.txt";
    jsonPath = "$.root[?(@.id)&&(@.user.screen_name)].quoted_status.entities.user_mentions[?(@.indices)&&(@.id_str)].id";
    num_threads = 64;
    pversion = 1;
    warmup_flag = 1;
    debug_flag = 1;
    percentage = 1;
    xmlPath=(char*)malloc(2000*sizeof(char));
    int ret = convertJSONPath(jsonPath, xmlPath);
    if(ret == 0)
    {
        printf("Input JSONPath query in config is not correct, please open the file and check it again!\n");
        return;
    }
    automata();
    ret = execute_query();
    if(ret==0)
    {
        printf("query execution exception, please check your input\n");
        return;
    }
    writing_results();
    if(debug_flag==1) print_debug_info();
}


void Test3()
{
    initialization();
    file_name = "bb.json";
    jsonPath = "$.root.products[?(@.sku)&&(@.productId)].categoryPath[?(@.name)].id";
    num_threads = 1;
    pversion = 1;
    warmup_flag = 1;
    debug_flag = 1;
    percentage = 1;
    xmlPath=(char*)malloc(2000*sizeof(char));
    int ret = convertJSONPath(jsonPath, xmlPath);
    if(ret == 0)
    {
        printf("Input JSONPath query in config is not correct, please open the file and check it again!\n");
        return;
    }
    automata();
    ret = execute_query();
    if(ret==0)
    {
        printf("query execution exception, please check your input\n");
        return;
    }
    writing_results();
    if(debug_flag==1) print_debug_info();
}

void Test4()
{
    initialization();
    file_name = "rows_uk.json";
    jsonPath = "$.root.meta.view.columns[?(@.id)&&(@.name)&&(@.cachedContents)].position";
    num_threads = 16;
    pversion = 0;
    warmup_flag = 1;
    debug_flag = 1;
    percentage = 1;
    xmlPath=(char*)malloc(2000*sizeof(char));
    int ret = convertJSONPath(jsonPath, xmlPath);
    if(ret == 0)
    {
        printf("Input JSONPath query in config is not correct, please open the file and check it again!\n");
        return;
    }
    automata();
    ret = execute_query();
    if(ret==0)
    {
        printf("query execution exception, please check your input\n");
        return;
    }
    writing_results();
    if(debug_flag==1) print_debug_info();
}

int main()
{
    Test0();
    /*Test1();
    Test2();
    Test3();
    Test4();*/
    return 0;
}

