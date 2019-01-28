#pragma once

#include "pass.hpp"
#include "Lexer/dfa.hpp"

namespace dragontooth {

class DFADump : public Pass{
public:
    DFADump() {}
    ~DFADump() {}

    virtual void Setup() {
        std::string path = cm->output();
        if (path == "") ec->Warn("No valid output path for DFADump.");
        output_path = path;
    }

    void Save(DFA* dfa);

    virtual IPassable* Execute(IPassable* data, IPassable* join_data);

private:
    std::string output_path;
};

}