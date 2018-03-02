#pragma once

#include <map>
#include <set>
#include <string>
#include "pass.hpp"
#include "Lexer/model_core.hpp"

namespace dragontooth {

class ActionParser : public Pass {
public:
    ActionParser() : scripts(1) {}
    virtual ~ActionParser() {}

    virtual void ScanPrerequisites(RegexModel* model);
    virtual void BuildActionList(RegexModel* model);
    virtual IPassable* Execute(IPassable* data, IPassable* join_data);

    // the arg of code is (arg << (8*code)), so 2 is this place: 00xx0000;
    // the next code with arg will be 1, then the 0
    enum action_code {ac_begin = 2, ac_nosave = 3, ac_ignore = 4, ac_common_action = 7}; 

protected:
    virtual int createCode(int now, int code, int arg);

    // key is the name of prerequistites, value is the id of the rules (begin from 0)
    std::map<std::string, std::set<int>> prename;
    // actual action code
    std::vector<int> actions;
    std::vector<std::string> scripts;
};


}