#include "xpath_builder.h"
#include "pass.hpp"
#include "Lexer/dfa_compressed.hpp"
#include "Lexer/ecset_converter.hpp"
#include "Lexer/regex_builder.hpp"
#include "Lexer/model_core.hpp"
#include "Lexer/dfa_mixer.hpp"
#include "Lexer/set_converter.hpp"
#include "Lexer/regex_model.hpp"

#include "xpath_model.h"
#include "xpath_verify.h"

extern "C" {
#include "parser_j.h"
#include "scanner_j.h"
}

using namespace std;
using namespace dragontooth;

extern "C" {

XPathNode* xpb_Analysis(const char* data) {
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

static void create_jsonpath_dfa(const char* json_path, RegexModel* model) {
    
    model->input_max = 10;
}

static JQ_DFA* create_dfa(DFACompressed* cpd_dfa) {

    return NULL;
}


JQ_DFA* xpb_Create(const char* json_path) {
    RegexModel* model = new RegexModel();
    create_jsonpath_dfa(json_path, model);

    SetConverter* start = new SetConverter();
    RegexBuilder* builder = new RegexBuilder();
    DFAMixer* mixer = new DFAMixer();
    start->next(builder)->next(mixer);

    RegexModel* data = start->ExecuteAll<RegexModel>(model);
    DFACompressed* dfa = (DFACompressed*)(data->main_dfa);
    dfa->print();

    delete model;
    delete start;
    delete builder;
    delete mixer;
    return NULL;
}


JQ_DFA* xpb_CreateMultiple(int num, const char* json_path[]) {
    // TODO: Implement merging multiple JSON Query DFAs
    return NULL;
}


}