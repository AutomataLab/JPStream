#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include "pass.hpp"
#include "Lexer/ecset_converter.hpp"
#include "Lexer/regex_builder.hpp"
#include "Lexer/model_core.hpp"
#include "Lexer/dfa_mixer.hpp"
#include "Lexer/dfa_compressed.hpp"
#include "Lexer/set_converter.hpp"
#include "Lexer/regex_model.hpp"
#include "dragontooth.h"

using namespace std;
using namespace dragontooth;


extern "C" {

void CallDemo() {
    RegexList* list = new RegexList();
    list->Add(new RegexChar(1));
    list->Add(new RegexSet("^\x02"));
    list->Add(new RegexChar(3));
    RegexModel* model = new RegexModel();
    model->input_max = 10;
    model->Add(list);

    SetConverter* start = new SetConverter();
    RegexBuilder* builder = new RegexBuilder();
    DFAMixer* mixer = new DFAMixer();
    start->next(builder)->next(mixer);

    RegexModel* data = start->ExecuteAll<RegexModel>(model);
    DFACompressed* dfa = (DFACompressed*)(data->main_dfa);
    dfa->print();

    DTLexerState* lexer = dtl_Create();
    
    lexer->m_base = dfa->m_base.data();
    lexer->m_next = dfa->m_next.data();
    lexer->m_check = dfa->m_check.data();
    lexer->m_default = dfa->m_default.data();
    lexer->m_stop_state = dfa->getStopState().data();
    lexer->bottom = dfa->getBottom();
    int state = 1, nextstate;
    for (int i = 0; i < 3; ++i) {
        nextstate = dtl_nextState(lexer, i+1, i+1);
        printf("now %d, %d : next %d\n", i+1, i+1, nextstate);
        state = nextstate;
    }
}

}