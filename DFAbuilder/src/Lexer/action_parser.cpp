#include "Lexer/action_parser.hpp"

namespace dragontooth {

void ActionParser::ScanPrerequisites(RegexModel* model) {
    for (int i = 0; i < model->size(); ++i) {
        auto& pres = model->at(i)->prerequisites;
        if (pres.size() == 0) {
            auto p = "default";
            if (prename.find(p) == prename.end()) 
                prename[p] = std::set<int>();
            prename[p].insert(i);
        } else {
            for (auto& p : pres) {
                if (prename.find(p) == prename.end())
                    prename[p] = std::set<int>();
                prename[p].insert(i);
            }
        }
    }
}

void ActionParser::BuildActionList(RegexModel* model) {
    for (int i = 0; i < model->size(); ++i) {
        auto& action = model->at(i)->actions;
        auto& script = model->at(i)->script;
        if (action != nullptr && script.length() == 0) {
            for (auto p : action->getData()) {
                int code = 0;
                if (p->getFunctionName() == "ignore") {
                    code = createCode(code, ac_ignore, 0);
                }
                if (p->getFunctionName() == "nosave") {
                    code = createCode(code, ac_nosave, 0);
                }
                if (p->getFunctionName() == "begin") {
                    if (p->getArgs().size() != 1) {
                        warn("The arguments length is not 1 for begin(x) action"); 
                    } else {
                        auto& name = p->getArgs()[0];
                        auto p = prename.find(name);
                        if (p == prename.end()) warn(std::string("The argument ") + name + "is not find");
                        code = createCode(code, ac_begin, std::distance(std::begin(prename), p));
                    }
                }
            }
        }
        // TODO: Custom Actions
    }
}

int ActionParser::createCode(int now, int code, int arg) {
    now |= (1<<code) << 24;
    if (code < 3) now |= arg << (8*code);
}

IPassable* ActionParser::Execute(IPassable* data, IPassable* join_data) {
    RegexModel* model = dynamic_cast<RegexModel*>(data);
    ScanPrerequisites(model);
    BuildActionList(model);
}

}  // namespace dragontooth