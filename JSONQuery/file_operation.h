int load_file(char* file_name); //load XML into memory(only used for sequential version)
int split_file(char* file_name, int n);  //split XML file into several parts and load them into memory
int write_file(char* file_name); //write output array into file
int write_file_with_predicates(char* file_name, int thread_numbers); //write output array into file (for predicates)
