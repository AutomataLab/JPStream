#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include "pass.hpp"
#include "Lexer/regex_parser.hpp"
#include "Lexer/ecset_converter.hpp"
#include "Lexer/regex_builder.hpp"
#include "Lexer/model_core.hpp"
#include "Lexer/dfa_mixer.hpp"
#include "Lexer/set_converter.hpp"
#include "Lexer/regex_model.hpp"
#include "dragontooth.h"

using namespace std;
using namespace dragontooth;

void CallDemo() {
    RegexList* list = new RegexList();
    list->Add(new RegexChar(1));
    auto* p = new RegexSet("^\x02"); 
    p->setOpt(RegexItem::re_repetition);
    list->Add(p);
    list->Add(new RegexChar(2));
    list->Add(new RegexChar(3));

    RegexList* list2 = new RegexList();
    list2->Add(new RegexChar(1));
    list2->Add(new RegexChar(3));

    RegexModel* model = new RegexModel();
    model->input_max = 10;
    model->Add(list);
    model->Add(list2);

    SetConverter* start = new SetConverter();
    RegexBuilder* builder = new RegexBuilder();
    DFAMixer* mixer = new DFAMixer();
    start->next(builder)->next(mixer);

    DFA* dfa = start->ExecuteAll<DFA>(model);
    dfa->print();

    DTLexerState* lexer = dtl_Create();
    
    lexer->m_base = dfa->m_base.data();
    lexer->m_next = (const uint32_t *)dfa->m_next.data();
    lexer->m_check = (const uint32_t *)dfa->m_check.data();
    lexer->m_default = dfa->m_default.data();
    lexer->m_token = dfa->stopState.data();
    lexer->bottom = dfa->Bottom;
    int state = 1, nextstate;
    for (int i = 0; i < 3; ++i) {
        nextstate = dtl_nextState(lexer, i+1, i+1);
        printf("now %d, %d : next %d\n", i+1, i+1, nextstate);
        state = nextstate;
    }
}

int main() {
    CallDemo();
    return 0;
}