

#pragma once

#include <stdint.h>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "Lexer/dfa.hpp"
#include "Lexer/regex_model.hpp"
#include "Lexer/equivalence_class.hpp"
#include "pass.hpp"
#include <cstdint>

namespace dragontooth {

/**
 * This structure is a item of Regex Model. It means an element in rule list.
 */
struct RegexModelItem {
    int id;
    std::string name;
    std::string value_name;
    std::string source;
    std::set<std::string> prerequisites; 
    std::string script;
    RegexItem* root;
    DFA* dfa;
    int rank;
    
    friend std::ostream& operator<<(std::ostream& out,
                                    const RegexModelItem& that) {
        out << that.name << " : " << that.source << std::endl;
        if (that.dfa) that.dfa->print();
        return out;
    }
};

/**
 * This is a Passable structure which is designed to hold the data using in
 * whole work flow in Lexer region
 */
class RegexModel : public IPassable {
public:
    RegexModel() : /* action_codes(1, 0), */ id_array(1), id_names(1) {}
    ~RegexModel() {}
    void Add(RegexItem* root) {
        auto p = new RegexModelItem{};
        p->rank = rules.size();
        p->root = root;
        rules.push_back(p);
    }
    void Add(const int id,
             const std::string& name, 
             const std::string& value_name,
             const std::string& source) {
        auto p = new RegexModelItem{id, name, value_name, source};
        p->rank = rules.size();
        rules.push_back(p);
        action_codes.push_back(0);
        id_array.push_back(id);
        id_names.push_back(name);
    }
    void Add(const int id,
             const std::string& name, 
             const std::string& value_name,
             const std::string& source,
             const std::vector<std::string>& prerequisites,
             const std::string& script) {
        auto p = new RegexModelItem{id, name, value_name, source};
        p->prerequisites.insert(prerequisites.begin(), prerequisites.end());
        p->script = script;
        p->rank = rules.size();
        rules.push_back(p);
        action_codes.push_back(0);
        id_array.push_back(id);
        id_names.push_back(name);
    }
    RegexModelItem* at(int i) const { return rules.at(i); }
    int size() const { return rules.size(); }

    RegexModelItem* find(const std::string& name) { 
        for (int i = 0; i < size(); ++i) {
            if (at(i)->name == name) return at(i);
        }
        return nullptr;
    }
    
    std::vector<uint32_t>& getActionCodes() { return action_codes; }
    std::vector<uint32_t>& getIDArray() { return id_array; }

    const std::vector<uint32_t>& getActionCodes() const { return action_codes; }
    const std::vector<uint32_t>& getIDArray() const { return id_array; }

    // @brief all input char need to be smaller than input_max
    int input_max = 256;

    // @brief the equivalence class, when the optional for ec is open
    EquivalenceClass* ec = nullptr;

    // @brief the main DFA after combine all DFAs
    DFA* main_dfa;

    friend std::ostream& operator<<(std::ostream& out, const RegexModel& that) {
        out << "RegexModel" << std::endl;
        out << "input_max = " << that.input_max << std::endl;
        for (int i = 0; i < that.size(); ++i) {
            out << that.id_names[i+1] << "(" << that.id_array[i+1] << ")" << std::endl;
            out << *(that.at(i));
            out << std::endl;
        }
        out << "Main DFA" << std::endl;
        if (that.main_dfa) that.main_dfa->print();
        return out;
    }

private:
    std::vector<RegexModelItem*> rules;
    std::vector<uint32_t> action_codes;
    std::vector<uint32_t> id_array;

    /* Debug Info */
public:
    const std::vector<std::string>& getIDNames() const { return id_names; }

private:
    std::vector<std::string> id_names;
};

}  // namespace dragontooth