#pragma once

#include <map>
#include <set>
#include <vector>
#include "Lexer/regex_model.hpp"
#include "pass.hpp"

namespace dragontooth {

/**
 * In DFA, the state 0 is the illegal state, first state from 1
 */
class DFA : public IPassable {
public:
	DFA() : input_max(256) {}
	DFA(int input_max) : input_max(input_max) {}
	virtual ~DFA() {}
	
	int getStateSum() { return m_base.size(); }

	// ========== State Map =========

	/// @brief used to get the next state
	int nextState(int s, regex_char a);

	/// @brief add a new edge
	void addEdge(int s, const std::vector<int>& objvec, int max);

	/// @brief check a state is the stop state, return the token id, or return 0 if not found
	int isStopState(int s) {
		return stopState[s];
	}

    void setStopState(int s,int t) {
        if (stopState.size() <= s) stopState.resize(s+1, 0);
        stopState[s] = t;
    }

	std::vector<int> m_default;
	std::vector<int> m_base;
	std::vector<int> m_next;
	std::vector<int> m_check;

	// @brief this stop state is the map of state number to actual token id
	std::vector<int> stopState;

	int input_max;
	int Top;
	int Bottom;

protected:

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

public:
	void printArrays();

	void print() {
		for (int i = 1; i< input_max; ++i) {
			printf("\t%d",i);
		}
		printf("\n");
		for (int i = 1; i< getStateSum(); ++i) {
			printf("s%d",i);
			int l;
			if ((l = isStopState(i)) != 0) printf("#%d",l);
			for (int j = 1; j< input_max; ++j) {
				printf("\ts%d",nextState(i,j));
			}
			printf("\n");
		}
	}
};

}  // namespace dragontooth