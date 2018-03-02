#pragma once

#include "Lexer/regex_model.hpp"
#include "pass.hpp"
#include "Lexer/dfa.hpp"

namespace dragontooth {

/**
 * RegexBuilder is the main stage in building the regex. It will prescan the
 * model and build the four main function: nullable, firstpos, lastpos and
 * followpos
 */
class RegexBuilder : public Pass {
public:
    RegexBuilder() : input_max(256) {}
    RegexBuilder(int input_max) : input_max(input_max) {}
    virtual ~RegexBuilder() {}

    void PrescanModel(RegexItem* item);
    DFA* CreateDFA(RegexItem* item);
    
    virtual IPassable* Execute(IPassable* data, IPassable* join_data);


    
protected:
    bool GetNextSet(const std::set<RegexItem*>& setNode, std::set<RegexItem*>& setNodeNext, regex_char a);

    // @brief all input char need to be smaller than input_max
	int input_max;

    // @brief listSet is the queue to create the different place
	std::vector< std::set<RegexItem*> > listSet;

};
}  // namespace dragontooth