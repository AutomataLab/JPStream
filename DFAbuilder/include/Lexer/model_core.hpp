

#pragma once

#include <stdint.h>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "Lexer/dfa.hpp"
#include "Lexer/regex_model.hpp"
#include "pass.hpp"

namespace dragontooth {

/**
 * This structure is a item of Regex Model. It means an element in rule list.
 */
struct RegexModelItem {
    RegexItem* root;
    DFA* dfa;

    friend std::ostream& operator<<(std::ostream& out,
                                    const RegexModelItem& that) {
        that.dfa->print();
        return out;
    }
};

/**
 * This is a Passable structure which is designed to hold the data using in
 * whole work flow in Lexer region
 */
class RegexModel : public IPassable {
public:
    void Add(RegexItem* root) {
        auto p = new RegexModelItem{root};
        rules.push_back(p);
    }
    
    RegexModelItem* at(int i) const { return rules.at(i); }
    int size() const { return rules.size(); }

    // @brief all input char need to be smaller than input_max
    int input_max = 256;

    // @brief the main DFA after combine all DFAs
    DFA* main_dfa;

    friend std::ostream& operator<<(std::ostream& out, const RegexModel& that) {
        out << "RegexModel" << std::endl;
        out << "input_max = " << that.input_max << std::endl;
        for (int i = 0; i < that.size(); ++i) {
            out << *(that.at(i));
        }
        out << "Main DFA" << std::endl;
        that.main_dfa->print();
        return out;
    }

private:
    std::vector<RegexModelItem*> rules;
};

}  // namespace dragontooth