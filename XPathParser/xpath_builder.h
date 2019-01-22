#ifndef __XPATH_BUILDER_H__
#define __XPATH_BUILDER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "dfa.h"
#include "xpath_model.h"

extern XPathNode* xpb_Analysis(const char* data);

extern JQ_DFA* xpb_Create(const char* json_path);

extern JQ_DFA* xpb_CreateMultiple(int num, const char* json_path[]);
    

#ifdef __cplusplus
}
#endif

#endif 