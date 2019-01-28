#include "config_manager.hpp"

using namespace std;
namespace dragontooth {

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