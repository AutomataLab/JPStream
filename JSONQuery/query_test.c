#include "query.h"
#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include "input.h"
#include "xpath_verify.h"
#include "xpath_builder.h"

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
    file_name = "../dataset/twitter.json";
    //file_name = "twitter_store1.txt"; 
    //jsonPath = "$.root[*].quoted_status.*.media[*].sizes.large.w";
    jsonPath = "$.root[?(@.id)].quoted_status.entities.user_mentions[?(@.indices)&&(@.id_str)].id";
   //jsonPath = "$.root[*].quoted_status.*.media[?(@.url)&&(@.media_url)].sizes.large.w";
    //jsonPath = "$.root[*].entities.hashtags[*].text"; 
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
    writing_results(); printf("output %d\n", outputs[2].top_output);
    if(debug_flag==1) print_debug_info(); 
}

void Test2()
{
    initialization();
    file_name = "../dataset/twitter.json";
    //file_name = "twitter_store1.txt";
    jsonPath = "$.root[?(@.text)&&(@.contributors)].id"; 
    //jsonPath = "$.root[?(@.id)&&(@.user.screen_name)].quoted_status.entities.user_mentions[?(@.indices)&&(@.id_str)].id";
    num_threads = 16;
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
    file_name = "../dataset/bb.json";
    jsonPath = "$.root.products[?(@.sku)&&(@.productId)].categoryPath[?(@.name)].id";
    //jsonPath = "$.root.products[?(@.sku)&&(@.productId)].categoryPath[?(@.name)].id";
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

void Test4()
{
    initialization();
    file_name = "../dataset/bb.json";
    //file_name = "rows_uk.json";
    jsonPath = "$.root.products[*].productId";
    //jsonPath = "$.root.meta.view.columns[?(@.id)&&(@.name)&&(@.cachedContents)].position";
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

void Test5()
{
    initialization();
    file_name = "../dataset/rowstest.json";
    jsonPath = "$.meta.view.columns[?(@.id)&&(@.name)&&(@.cachedContents)].position";
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

void Test6()
{
    initialization();
    file_name = "../dataset/wiki.json";
    jsonPath = "$.root[*].title";
    num_threads = 16;
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

void Test7()
{
    initialization();
    file_name = "../dataset/wiki.json";
    jsonPath = "$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property";
    num_threads = 64;
    pversion = 1;
    warmup_flag = 1;
    debug_flag = 1;
    percentage = 10;
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

void Test8()
{
    initialization();
    file_name = "../dataset/random.json";
    jsonPath = "$.root[?(@.index&&@.guid)].friends[?(@.name)].id";
    num_threads = 64;
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

void Test9()
{
    JQ_DFA* dfa = xpb_Create("$.root[?(@.index && @.guid)].friends[?(@.name)].id");
    printf("1 (3) => %d\n", jqd_nextState(dfa, 1, 3));
}

int main()
{
    Test0();
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
    Test7();
    Test8();
    Test9();
    return 0;
}

