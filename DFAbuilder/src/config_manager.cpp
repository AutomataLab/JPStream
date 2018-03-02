#include "config_manager.hpp"
#include "cmdline.h"

using namespace std;
namespace dragontooth {

int ConfigManager::ParseArgs(int argc, char** argv) {
    cmdline::parser parser;
    parser.add<string>("lexer", 'l', "lexer scheme for lexical analysis", false, "normal", cmdline::oneof<string>("none", "normal", "parallel"));
    parser.add<string>("parser", 'p', "parser scheme for gramma analysis", false, "LALR", cmdline::oneof<string>("none", "LALR", "DEPP", "EXLALR"));
    parser.add<string>("output", 'o', "output file for the final code");
    parser.add("utf8", '\0', "enable uft8 support", false, true);
    parser.add("estring", '\0', "enable estring support", false, false);
    parser.add("eqclass", 'e', "enable equivalence class support", false, true);
    parser.add("minidfa", 'm', "enable minimize dfa in lexer", false, true);
    parser.add("compresstable", 'c', "enable minimize dfa in lexer", false, true);
}

void ConfigManager::initDefault() {
    // components
    values["lexical_analysis"] = "normal"; // it could be none, normal or parallel
    values["gramma_analysis"] = "LALR"; // it could be none, LALR, DEPP, EXLALR, or others in the feature
    
    values["output"] = "";

    // Lexer
    switchs["utf8"] = false;
    switchs["estring"] = false;
    switchs["equivalence_class"] = true;
    switchs["minimize_dfa"] = true;
    switchs["compress_table"] = true;

    // LALR
    switchs["actions"] = true; // support actions in LALR 

    switchs["generate_code"] = true; // if true, then it will generate runtime with table, otherwise only generate the code of 
    switchs["pass_hold_pointer"] = true; // if true, the data pointer output by each pass will be hold by them. It will be automatic release when the pass release.
}

}