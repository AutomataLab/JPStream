#include "Lexer/dfa_dump.hpp"

namespace dragontooth {

void DFADump::Save(DFA* dfa) {
    // TODO: save the DFA
}

IPassable* DFADump::Execute(IPassable* data, IPassable* join_data) {
    DFA* dfa = dynamic_cast<DFA*>(data);
    Save(dfa);
    return data;
}

}