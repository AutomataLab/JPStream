#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include "pass.hpp"
#include "Lexer/dfa_compressed.hpp"
#include "Lexer/ecset_converter.hpp"
#include "Lexer/regex_builder.hpp"
#include "Lexer/model_core.hpp"
#include "Lexer/dfa_mixer.hpp"
#include "Lexer/set_converter.hpp"
#include "Lexer/regex_model.hpp"
#include "dragontooth.h"

using namespace std;
using namespace dragontooth;

void CallDemo(RegexModel* model) {
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

void Test1() {
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
    CallDemo(model);
}

void Test2() {
    RegexList* list = new RegexList();
    list->Add(new RegexChar(1));
    list->Add(new RegexChar(2));
    list->Add(new RegexChar(3));
    list->Add(new RegexChar(4));

    RegexList* list2 = new RegexList();
    list2->Add(new RegexChar(1));
    list2->Add(new RegexChar(2));
    list2->Add(new RegexChar(3));
    list2->Add(new RegexChar(5));

    RegexList* list3 = new RegexList();
    list3->Add(new RegexChar(1));
    list3->Add(new RegexChar(2));
    list3->Add(new RegexChar(3));
    list3->Add(new RegexChar(4));
    list3->Add(new RegexChar(5));
    list3->Add(new RegexChar(6));
    list3->Add(new RegexChar(7));
    list3->Add(new RegexChar(8));
    list3->Add(new RegexChar(9));


    RegexModel* model = new RegexModel();
    model->input_max = 10;
    model->Add(list);
    model->Add(list2);
    model->Add(list3);
    CallDemo(model);
}

int main() {
    Test1();
    Test2();
    return 0;
}