#ifndef __INPUT_H__
#define __INPUT_H__

int get_parameters_from_config(char* file_name); //gather all parameters from configuration file
int convertJSONPath(char* jsonPath, char* xmlPath); //convert JSONPath query into an inner representation

#endif // !__INPUT_H__
