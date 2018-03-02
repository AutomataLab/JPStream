#pragma once
#include "pass.hpp"
#include "Lexer/model_core.hpp"

namespace dragontooth {

/**
 * @brief DFAMixer is a stage which can combine multiple DFAs into one DFA.
 */
class DFAMixer : public Pass {
public:
    DFAMixer() {}
    virtual ~DFAMixer() {}

    virtual void Combine(RegexModel* model);
    virtual IPassable* Execute(IPassable* data, IPassable* join_data);

protected:
    virtual void addStopState(RegexModel* model, const std::vector<int>& newvec, int p);
    DFA* main_dfa;
};

}