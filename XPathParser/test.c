#include "xpath_model.h"
#include "parser.h"
#include "scanner.h"

XPathNode* Analysis(const char* data) {
    XPathNode* root;
    yyscan_t sc;
    int res;
    xxlex_init(&sc);
    YY_BUFFER_STATE buffer = xx_scan_string(data, sc);
    res = xxparse(sc, &root);
    xx_delete_buffer(buffer, sc);
    xxlex_destroy(sc);
    return model;
}

int main() {

    
}