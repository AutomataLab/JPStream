#pragma once

#include <map>
#include <set>
#include <vector>
#include <cstdint>
#include "Lexer/regex_model.hpp"
#include "Lexer/dfa.hpp"
namespace dragontooth {

/**
 * In DFA, the state 0 is the illegal state, first state from 1
 */
class DFASimple : public DFA {
public:
	DFASimple() {}
	DFASimple(int input_max) : DFA(input_max) {}
	virtual ~DFASimple() {}
	
	/// @brief used to get the next state
	virtual int nextState(int s, regex_char a);

	/// @brief add a new edge
	virtual void addEdge(int s, const std::vector<int>& objvec, int max);

	// virtual uint32_t getStateSum() { return state_num; }
	// virtual uint32_t getInputSize() { return input_max; }
	virtual void printArrays();

protected:
	// ========== State Map =========
	std::vector<uint32_t> m_table;
};

}  // namespace dragontooth