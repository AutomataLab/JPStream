#include "Lexer/dfa_simple.hpp"
#include "limits.h"

using namespace std;
namespace dragontooth {

int DFASimple::nextState(int s, regex_char ch) {
    return m_table[s* input_max + ch];
}

void DFASimple::addEdge(int s, const std::vector<int>& objvec, int max) {
    if (m_table.size() <= max*input_max)
        m_table.resize((max+1)*input_max, 0);

    for (int i = 0; i < input_max; ++i) 
        m_table[s * input_max + i] = objvec[i];
    state_num++;
}


void DFASimple::printArrays() {
    cout << "input_max: " << input_max << endl;
    cout << "state_num: " << state_num << endl;

    cout << "m_table:" << m_table.size() <<  endl;
    for (auto p : m_table) { cout << p << ' '; }
    cout << endl;
}

}  // namespace dragontooth