#include "Lexer/regex_builder.hpp"
#include <algorithm>
#include "Lexer/regex_model.hpp"
#include "Lexer/model_core.hpp"
#include "Lexer/dfa_simple.hpp"

using namespace std;
namespace dragontooth {

void RegexBuilder::PrescanModel(RegexItem* item) { item->BuildAll(); }


IPassable* RegexBuilder::Execute(IPassable* data, IPassable* join_data) {
    RegexModel* model = dynamic_cast<RegexModel*>(data);
    input_max = model->input_max;
    for (int i = 0; i < model->size(); ++i) {
        auto p = model->at(i);
        PrescanModel(p->root);
        // p->root->printTree(0);
        p->dfa = CreateDFA(p->root);
    }
    return model;
}


bool RegexBuilder::find_keepout(const set<RegexItem*>& now_set) {
    for (auto p : now_set) {
        if (p->keepout == true) {
            return true;
        }
    }
    return false;
}

DFA* RegexBuilder::CreateDFA(RegexItem* item) {
    item->addTerminator();
    DFA* dfa = new DFASimple(input_max);
    int p = 0;
    // initialize queue and put the firstpos of root as the first element.
    RegexChar ch;
    ch.followpos = item->firstpos;
    set<RegexItem*> begin;
    begin.insert(&ch);
    listSet.clear();
    listSet.push_back(begin);
    vector<int> transform_buffer;
    while (p < listSet.size()) {
        // This line will let all the set contain a keepout stop finding next edge
        if (find_keepout(listSet[p])) { ++p; continue; }

        transform_buffer.clear();
        transform_buffer.resize(input_max);
        for (int a = 1; a < input_max; ++a) {
            set<RegexItem*> nextset;
            bool isTerminator =
                GetNextSet(listSet[p], nextset, a);  // set the nextset
            if (nextset.size() != 0) {
                // found a set
                // check this node if it's in the list
                auto i = find(listSet.begin(), listSet.end(), nextset);
                if (i == listSet.end()) {
                    listSet.push_back(nextset);
                    // if the nextset has a stop state, add it to the map
                    if (isTerminator)
                        // we didn't know the token id now, it will be set later
                        dfa->setStopState(listSet.size(), 1);
                    // add a new transform
                    transform_buffer[a] = listSet.size();
                } else {
                    transform_buffer[a] = (i - listSet.begin()) + 1;
                }
            }
        }
        dfa->addEdge(p + 1, transform_buffer, listSet.size());
        ++p;
    }
    return dfa;
}

bool RegexBuilder::GetNextSet(const std::set<RegexItem*>& setNode,
                              std::set<RegexItem*>& setNodeNext, regex_char a) {
    bool isTerminator = false;
    for (auto p : setNode)
        for (auto f : p->followpos)
            if (f->isEdge(a)) {
                if (f->isTerminator) isTerminator = true;
                setNodeNext.insert(f);
            }
    return isTerminator;
}

}  // namespace dragontooth