#include "Lexer/dfa_mixer.hpp"
#include <algorithm>
#include <vector>
#include "Lexer/dfa_compressed.hpp"

using namespace std;

namespace dragontooth {

int DFAMixer::find_states(vector<vector<int>>& statelist, int begin, int i, vector<int>& newvec) {
    // find the begin state
    auto beg = statelist.begin()+begin;
    auto end = statelist.begin()+begin+1;
    auto q = find(beg, end, newvec);
    if (q != end) return begin;

    // find other part
    if (i < statelist.size()) {
        q = find(statelist.begin()+i, statelist.end(), newvec);
        if (q != statelist.end()) return q - statelist.begin();
    }
    // can not find
    return -1;
}

void DFAMixer::CombineSpecial(RegexModel* model, vector<vector<int>>& statelist, int begin, int top) {
    int p = 0, i = begin;
    while (i < statelist.size()) {
        vector<int> transform_buffer(model->input_max);
        for (int c = 1; c < model->input_max; ++c) {
            vector<int> newvec;
            bool all_zero = true;
            for (int j = 0; j < model->size(); ++j) {
                auto r = model->at(j);
                DFA* dfa = r->dfa;
                int nowstate = statelist[i][j];
                int nextstate = 0;
                if (nowstate != 0) {
                    nextstate = dfa->nextState(nowstate, c);
                    if (nextstate != 0) all_zero = false;
                }
                newvec.push_back(nextstate);
            }
            if (all_zero) {
                transform_buffer[c] = 0;
            } else {
                int q = find_states(statelist, begin, top, newvec);
                if (q == -1) {
                    statelist.push_back(newvec);
                    p = statelist.size()-1;
                    addStopState(model, newvec, p);
                } else
                    p = q;
                transform_buffer[c] = p;
            }
        }
        main_dfa->addEdge(i, transform_buffer, statelist.size());
        if (i == begin) i = top; else ++i;
    }
}


void DFAMixer::Combine(RegexModel* model) {
    vector<int> states;
    vector<vector<int>> statelist;
    main_dfa = new DFACompressed(model->input_max);

    for (int i = 0; i < model->size(); ++i) states.push_back(0);
    statelist.push_back(states); // statelist[0]

    set<string> prenames;
    for (int i = 0; i < model->size(); ++i) {
        const auto& pre = model->at(i)->prerequisites;
        for (auto p : pre) 
            if (p != "default")
                prenames.insert(p);
    }
    // This is for default
    states.clear();
    for (int i = 0; i < model->size(); ++i) {
        const auto& pre = model->at(i)->prerequisites;
        if (pre.empty() || pre.find("default") != pre.end()) 
            states.push_back(1);
        else
            states.push_back(0);
    }
    statelist.push_back(states);  // statelist[1]
    addStopState(model, states, 1);

    int count = 2;
    // This is for other prerequisites
    for (auto& name : prenames) {
        states.clear();
        for (int i = 0; i < model->size(); ++i) {
            const auto& pre = model->at(i)->prerequisites;
            if (!pre.empty() && pre.find(name) != pre.end()) 
                states.push_back(1);
            else
                states.push_back(0);
        }
        statelist.push_back(states);  // statelist[2] ..
        addStopState(model, states, count++);
    }
    
    int size = statelist.size();
    CombineSpecial(model, statelist, 1, size);

    for (int i = 2; i < size; ++i) {
        CombineSpecial(model, statelist, i, statelist.size());
    }

    model->main_dfa = main_dfa;
}

void DFAMixer::addStopState(RegexModel* model, const std::vector<int>& newvec, int p) {
    for (int i = newvec.size()-1; i >= 0; --i) { // 颠倒优先级
        if (newvec[i] != 0) {
            auto r = model->at(i);
            DFA* dfa = r->dfa;
            if (dfa->isStopState(newvec[i]) != 0) {
                main_dfa->setStopState(p, i+1);
                return;  // 这句话必须要有，使得自动机能够具有从上到下所有rule的优先级，可以从上到下依次匹配
            }
        }
    }
}

IPassable* DFAMixer::Execute(IPassable* data, IPassable* join_data) {
    RegexModel* model = dynamic_cast<RegexModel*>(data);
    Combine(model);
    return model;
}

}  // namespace dragontooth