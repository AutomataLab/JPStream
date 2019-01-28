#pragma once
#include "pass.hpp"
#include "Lexer/model_core.hpp"

namespace dragontooth {

/**
 * @brief DFAMixer is a stage which can combine multiple DFAs into one DFA.
 * In this stage, input is regex model, output is also regex model.
 * But the main DFA in the model have been generated.
 */
class DFAMixer : public Pass {
public:
    DFAMixer() {}
    virtual ~DFAMixer() {}

    /**
     * @brief This function will combine multiple DFAs in the regex model and generate the main DFA
     */
    virtual void Combine(RegexModel* model);

    virtual IPassable* Execute(IPassable* data, IPassable* join_data);
    
protected:
    int find_states(std::vector<std::vector<int>>& statelist, int begin, int i, std::vector<int>& newvec);
    void CombineSpecial(RegexModel* model, std::vector<std::vector<int>>& statelist, int begin, int i);
    virtual void addStopState(RegexModel* model, const std::vector<int>& newvec, int p);
    DFA* main_dfa;
};

}