#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>
#include "pass.hpp"
#include "Lexer/charset.hpp"
#include "Lexer/regex_model.hpp"

namespace dragontooth {

/**
 * EquivalenceClass is used to translate normal charset to a small charset. And
 * it cost very cheap in runtime (only a table lookup).
 */
class EquivalenceClass : public IPassable {
public:
    EquivalenceClass();
    virtual ~EquivalenceClass();

    void Add(regex_char ch);
    void Add(const std::vector<regex_char>& str);
    void Add(CharSet& charset);
    void Rearrage();

    regex_char makeChar(regex_char ch);
    std::vector<regex_char> makeString(const std::vector<regex_char>& str);
    std::set<regex_char> makeSet(CharSet& charset);
    regex_char getClass(regex_char);

    regex_char getSum() { return eclass_sum+1; }
    int getInputMax() { return vec.size(); }
    std::vector<regex_char>& getVector() { return vec; }
protected:
    // set the vector from p to q
    void insert(regex_char k);
    void insert(regex_char k, std::map<regex_char, regex_char>& m);
    void insert(regex_char p, regex_char q,
                std::map<regex_char, regex_char>& m);

    regex_char eclass_sum = 1;
    std::vector<regex_char> vec;
};

}  // namespace dragontooth