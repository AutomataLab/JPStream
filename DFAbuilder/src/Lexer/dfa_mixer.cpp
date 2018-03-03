#include "Lexer/dfa_mixer.hpp"
#include <algorithm>
#include <vector>
#include "Lexer/dfa.hpp"

using namespace std;

namespace dragontooth {

void DFAMixer::Combine(RegexModel* model) {
    // TODO: testing, support prerequisites
    vector<int> states;
    vector<vector<int>> statelist;
    main_dfa = new DFA(model->input_max);

    for (int i = 0; i < model->size(); ++i) states.push_back(0);
    statelist.push_back(states);
    states.clear();
    for (int i = 0; i < model->size(); ++i) states.push_back(1);
    statelist.push_back(states);
    addStopState(model, states, 1);

    int i = 0, p = 0;
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
                auto i = find(statelist.begin(), statelist.end(), newvec);
                if (i == statelist.end()) {
                    statelist.push_back(newvec);
                    p = statelist.size()-1;
                    addStopState(model, newvec, p);
                } else
                    p = (i - statelist.begin());
                transform_buffer[c] = p;
            }
        }
        main_dfa->addEdge(i, transform_buffer, statelist.size());
        ++i;
    }
    model->main_dfa = main_dfa;
}

void DFAMixer::addStopState(RegexModel* model, const std::vector<int>& newvec,
                            int p) {
    for (int i = 0; i < newvec.size(); ++i) {
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
    if (model->size() == 1) return model->at(0)->dfa;
    Combine(model);
    return model->main_dfa;
}

}  // namespace dragontooth