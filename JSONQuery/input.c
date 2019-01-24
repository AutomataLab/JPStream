#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "basic.h"
#include "global.h"
#include "input.h"


/*********************************************************
Function: void get_parameters_from_config(char* config_file);
Input: config_file--the name of configuration file
Description: gather all parameters from configuration file
Return: 1--success, 0--error
**********************************************************/
int get_parameters_from_config(char* config_file)
{
    //read some parameters from configuration file
	FILE *fp;
	char* buf=(char*)malloc(2*MAX_FILE*sizeof(char));
	char seps[] = "="; 
	char *token_line=NULL; 
	int line=0;
	if((fp = fopen(config_file,"r")) == NULL)
    {
        printf("There is something wrong with the config file, we can not load it. Please check whether it is placed in the right place.\n");
    	return 0;
    }
    else{
    	while(fgets(buf,2*MAX_FILE,fp) != NULL)
    	{
    		token_line = strtok(buf, seps); 
    		if(strcmp(token_line,"File_Name")==0)
    		{
    			token_line=strtok(NULL,seps);
    			if(token_line!=NULL)
    			{
    				file_name=malloc(MAX_SIZE*sizeof(char));
                    file_name=strcpy(file_name,"./");
    				file_name=strcpy(file_name,token_line);
    				file_name[strlen(file_name)-2]='\0'; 
				}
			}
    		else if(strcmp(token_line,"JSONPath")==0)
    		{
    			token_line=strtok(NULL,seps);
    			if(token_line!=NULL)
    			{
    				jsonPath=malloc(2*MAX_FILE*sizeof(char));
    				jsonPath=strcpy(jsonPath,token_line);
    				jsonPath[strlen(jsonPath)-2]='\0';   
				}
			}
			else if(strcmp(token_line,"version(0--sequential, 1--parallel)")==0)
			{
				token_line=strtok(NULL,seps);
				if(token_line!=NULL)
				{
					sscanf(token_line,"%d",&choose);
				}
			}
			else if(strcmp(token_line,"number-of-threads(no less than 1 and no more than 100)")==0)
			{
				token_line=strtok(NULL,seps);
				if(token_line!=NULL)
				{
					sscanf(token_line,"%d",&num_threads);
				}
			}
			else if(strcmp(token_line,"pversion(0--JPStreamR, 1--JPStreamR+)")==0)
			{
				token_line=strtok(NULL,seps);
				if(token_line!=NULL)
				{
					sscanf(token_line,"%d",&pversion);
				}
			}
            else if(strcmp(token_line, "warm_up_cpu(0--no, 1--yes)")==0)
            {
                token_line = strtok(NULL,seps);
                if(token_line!=NULL)
                {
                    sscanf(token_line,"%d",&warmup_flag);
                }
            }
            else if(strcmp(token_line, "print_debug_info(0--no, 1--yes)")==0)
            {
                token_line = strtok(NULL,seps);
                if(token_line!=NULL)
                {
                    sscanf(token_line,"%d",&debug_flag);
                }
            }
            else if(strcmp(token_line, "statistic_info(0--no, 1--yes)")==0)
            {
                token_line = strtok(NULL,seps);
                if(token_line!=NULL)
                {
                    sscanf(token_line,"%d",&statistic_flag);
                }
            }
            else if(strcmp(token_line, "profiling_percentage(data constraint, no less than 1 and no more than 100)")==0)
            {
                token_line = strtok(NULL,seps);
                if(token_line!=NULL)
                {
                    sscanf(token_line,"%d",&percentage);
                }
            }
		}
	}
	free(buf);
	fclose(fp);
 
    printf("Thanks for using JPStream! Your file name is %s\n\n",file_name);
    //checking all parameters
    if(file_name==NULL)
    {
    	printf("The File_Name in config can not be empty, please open the file and check it again!\n");
    	return 0;
	}
	if(jsonPath==NULL)
	{
		printf("The JSONPath in config can not be empty, please open the file and check it again!\n");
    	return 0;
	}
    if(choose!=0&&choose!=1)
    {
    	printf("The number of version(0--sequential, 1--parallel) in config is not correct, please open the file and check it again!\n");
    	return 0;
	}
    
    if(choose==1)
	{
        if((num_threads<1)||(num_threads>100))
        {
    	    printf("The number-of-threads(no less than 1 and no more than 100) in config is not correct, please open the file and check it again!\n");
    	    return 0;
	    }
	}
	
	if(pversion!=0&&pversion!=1)
	{
		printf("The pversion(0--VLDB'13, 1--our method) in config is not correct, please open the file and check it again!\n");
    	return 0;
	}
	
	if(warmup_flag!=0&&warmup_flag!=1)
	{
		printf("The warm_up_cpu(0--no, 1--yes) in config is not correct, please open the file and check it again!\n");
    	return 0;
	}
	
	if(debug_flag!=0&&debug_flag!=1)
	{
		printf("The print_debug_info(0--no, 1--yes) in config is not correct, please open the file and check it again!\n");
    	return 0;
	}
	
	if(statistic_flag!=0&&statistic_flag!=1)
	{
		printf("The statistic_info(0--no, 1--yes) in config is not correct, please open the file and check it again!\n");
    	return 0;
	}
        if((percentage<1)||(percentage>100))
        {
                printf("The statistic_info(0--no, 1--yes) in config is not correct, please open the file and check it again!\n");
        return 0;
        }
  
        xmlPath=malloc(2*MAX_FILE*sizeof(char)); 
        int ret = convertJSONPath(jsonPath, xmlPath);
        if(ret == 0)
        {
            printf("Input JSONPath query in config is not correct, please open the file and check it again!\n");
            return 0; 
        }	
	return 1;
}

/***************************************************************
Function: void convertJSONPath(char* jsonPath, char* xmlPath);
Description: convert JSONPath query into an inner representation
****************************************************************/
int convertJSONPath(char* jsonPath, char* xmlPath)
{
	//pt.top_predicate = -1;
	//ai.top_index = -1;
	int length = strlen(jsonPath);
	if(length<=1) return 0;
	if(jsonPath[0]!='$') return 0;
	int i;
	char temp[500];
	int count_temp=-1;
	int tag_array=0;
	int tag_array_element = 0;
	char data[10];
	int count_data=-1;
	
	int start_predicate = -1;
	int end_predicate = -1;
	int has_predicate = 0;
   	for(i=1; i<length;i++)
	{
		//printf("char %c\n",jsonPath[i]);
		if(jsonPath[i]==' ')
            continue;
                
		if(tag_array==1&&jsonPath[i]!=']'&&jsonPath[i]!='*'&&jsonPath[i]!=':'){
		    tag_array_element = 1;
	    }
		if(jsonPath[i]=='.') temp[++count_temp]='/';
		else if(jsonPath[i]=='[')
		{
			temp[++count_temp]='/';
			temp[++count_temp]='a';
			temp[++count_temp]='r';
			temp[++count_temp]='r';
			temp[++count_temp]='a';
			temp[++count_temp]='y';
			temp[++count_temp]='[';
			start_predicate = i;
			tag_array=1;
		}
		else if(jsonPath[i]==']')
		{
			//printf("predicate %d\n",has_predicate);
			if(temp[count_temp]==')') count_temp--;
			if(tag_array_element == 0)
			{
				--count_temp; 
			}
			else
			{
				temp[++count_temp] = jsonPath[i];
				if(has_predicate == 1)
				{
					end_predicate = i;
					//printf("predicate string %s\n", substring(jsonPath,start_predicate, end_predicate+1));
					//strcopy(substring(jsonPath,start_predicate, end_predicate+1),pt.predicates[++pt.top_predicate]);
					has_predicate = 0;
				}
			}
			tag_array=0;
			tag_array_element = 0;
			if(count_data>-1)
			{
				data[++count_data] = '\0';
				count_data = -1;
			}
		}
		else if(tag_array==1)
		{
			if(jsonPath[i]=='@'||jsonPath[i]=='?')
			{
				i = i+1;
				//printf("predicate %d\n",has_predicate);
				if(has_predicate == 0)
				{
					has_predicate = 1; //printf("yespre\n");
				}
			}
			else if(jsonPath[i]=='&'&&jsonPath[i+1]=='&')
			{
				temp[++count_temp] = ' ';
				temp[++count_temp] = 'a';
				temp[++count_temp] = 'n';
				temp[++count_temp] = 'd';
				temp[++count_temp] = ' ';
				i = i+1;
			}
			else if(jsonPath[i]=='|'&&jsonPath[i+1]=='|')
			{
				temp[++count_temp] = ' ';
				temp[++count_temp] = 'o';
				temp[++count_temp] = 'r';
				temp[++count_temp] = ' ';
				i = i+1;
			}
			else if(jsonPath[i]!='*'&&jsonPath[i]!=':') 
			{
				temp[++count_temp] = jsonPath[i];
			}
			//if(jsonPath[i]!=':')
			  //  data[++count_data]=jsonPath[i];
		}
		else
		{
			temp[++count_temp]=jsonPath[i];
		}
	}
	temp[++count_temp] = '\0';
	strcopy(temp,xmlPath);
        if(debug_flag==1) printf("xpath %s\n", xmlPath);
	return 1;
}
