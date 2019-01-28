#ifndef __JSONPATH_PARSER_H__
#define __JSONPATH_PARSER_H__

#include "jsonpath_model.h"

#ifdef __cplusplus
extern "C" {
#endif

extern JSONPathNode* jpp_Analysis(const char* data);

#ifdef __cplusplus
}
#endif

#endif