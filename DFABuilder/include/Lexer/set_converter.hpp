#pragma once

#include "regex_model.hpp"
#include "pass.hpp"
#include <vector>

namespace dragontooth {

/**
 * @brief Simple Set Converter, without EquivalenceClass Converter, so the max charset support 256
 */ 
class SetConverter : public Pass {
public:
    SetConverter() {}
    virtual ~SetConverter() {}

    virtual void Convert(RegexSet* itemset);
    virtual void ConvertAll(RegexItem* root) {
        traverses_model(root);
    }
    
    virtual IPassable* Execute(IPassable* data, IPassable* join_data);

protected:
    // for each node in this tree, if it's a set, then run convert
    void traverses_model(RegexItem* root);

    void insert(regex_char k, bool negate, std::vector<bool>& cvs);
    void insert(regex_char p, regex_char q, bool negate, std::vector<bool>& cvs);
    
};
}  // namespace dragontooth