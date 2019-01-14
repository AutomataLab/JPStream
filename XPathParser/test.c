#include "xpath_model.h"
#include "xpath_verify.h"
#include "parser_j.h"
#include "scanner_j.h"

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
    xpn_PrintJSON(root);

    root = Analysis("$[?(@.a)]");
    xpn_PrintJSON(root);

    root = Analysis("$[?(@.a&&(@.b||@.c))]");
    xpn_PrintJSON(root);

    root = Analysis("$[?((@.a||@.d&&(@.e||@.f))&&(@.b||@.c))]");
    xpn_PrintJSON(root);
    return 0;
}
