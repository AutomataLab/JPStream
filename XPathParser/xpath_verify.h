#ifndef __XPATH_VERIFY_H__
#define __XPATH_VERIFY_H__

#include "xpath_model.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum XPathValueType {
    xvt_number, xvt_string, xvt_boolean
} XPathValueType;

typedef struct XPathValue {
    union {
        double number;
        char* string;
        bool boolean;
    };
    XPathValueType vtype;
} XPathValue;

typedef struct XPathKeyValuePair {
    char* key;
    XPathValue value;
} XPathKeyValuePair;

/**
 * Before calling this function, please replace the '@.proptery' subtree to a single node - xnt_variable
 * You can create it by calling @xpn_CreateVariable(var_name)
 * When I calculate the result, I will try to search the table to find the values of those variable nodes.
 */
static inline XPathValue xpv_Calculate(XPathNode* node, XPathKeyValuePair* table) {
    
}





#ifdef __cplusplus
}
#endif


#endif