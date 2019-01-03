#pragma once

#include <map>
#include <set>
#include <vector>
#include "Lexer/regex_model.hpp"
#include "Lexer/dfa.hpp"
#include <cstdint>

namespace dragontooth {

/**
 * In DFACompressed, the state 0 is the illegal state, first state from 1
 * It will compress the data structure of DFA automatically and reduce the space
 */
class DFACompressed : public DFA {
public:
	DFACompressed() {}
	DFACompressed(int input_max) : DFA(input_max) {}
	virtual ~DFACompressed() {}
	

	/// @brief used to get the next state
	int nextState(int s, regex_char a);

	/// @brief add a new edge
	void addEdge(int s, const std::vector<int>& objvec, int max);

	std::vector<int32_t> m_default;
	std::vector<int32_t> m_base;
	std::vector<uint32_t> m_next;
	std::vector<uint32_t> m_check;

	uint32_t getBottom() { return Bottom; }
	virtual uint32_t getStateSum() { return m_base.size()-1; }

	virtual void printArrays();

protected:
	uint32_t Top = 0;
	uint32_t Bottom = 0;

	void createNewArea(int base, int s, const std::vector<int>& objvec);
	void createLinkArea(int base, int s, const std::vector<int>& objvec);

	bool compareState(int s, const std::vector<int>& vec) {
		int counter = 10;
		if (m_default[s] > 0) return false;
		for (int a = 1; a < vec.size(); ++a) {
			if (vec[a] != nextState(s, a)) counter--;
			if (counter <= 0) return false;
		}
		return true;
	}

	int getValidSize(const std::vector<int>& vec, int max, int& most_frq) {
		std::vector<int> sum(m_default.size()+1);
		for (int q : vec) {
			sum[q]++;
		}
		int most = 0;
		for (int a = 0; a < sum.size(); ++a) {
			int q = sum[a];
			if (q > most) { most = q; most_frq = a; }
		}
		return vec.size() - most;
	}

	void checkSpace(int i) {
		if (i >= m_next.size()) {
            m_next.resize(i + 1);
            m_check.resize(i + 1);
        }
	}
};

}  // namespace dragontooth