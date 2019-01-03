#include "xpath_model.h"
#include "parser.h"
#include "scanner.h"

XPathNode* Analysis(const char* data) {
    XPathNode* root;
    yyscan_t sc;
    int res;
    jlex_init(&sc);
    YY_BUFFER_STATE buffer = j_scan_string(data, sc);
    res = jparse(sc, &root);
    j_delete_buffer(buffer, sc);
    jlex_destroy(sc);
    return root;
}

int main() {
    XPathNode* root = Analysis("$.store.book[?(@.price < 10)].title");
    return 0;
}