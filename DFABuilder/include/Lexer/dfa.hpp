#pragma once

#include <map>
#include <set>
#include <vector>
#include "Lexer/regex_model.hpp"
#include <cstdint>

namespace dragontooth {

/**
 * In DFA, the state 0 is the illegal state, first state from 1
 */
class DFA {
public:
	/// @brief used to get the next state
	virtual int nextState(int s, regex_char a) = 0;

	/// @brief add a new edge
	virtual void addEdge(int s, const std::vector<int>& objvec, int max) = 0;

	/// @brief check a state is the stop state, return the token id, or return 0 if not found
	virtual int isStopState(int s) {
		if (s >= stopState.size()) return 0;
		return stopState[s];
	}

    virtual void setStopState(int s,int t) {
        if (stopState.size() <= s) stopState.resize(s+1, 0);
        stopState[s] = t;
    }

	virtual const std::vector<int32_t>& getStopState() {
        if (stopState.size() <= state_num) stopState.resize(state_num+1, 0);
		return stopState;
	}

	virtual uint32_t getStateSum() { return state_num; }
	virtual uint32_t getInputSize() { return input_max; }

protected:
	DFA() : input_max(256) {}
	DFA(int input_max) : input_max(input_max) {}
	virtual ~DFA() {}

	int input_max;
	uint32_t state_num = 1;

	std::vector<int32_t> stopState;

/** Debug Functions */

public:
	virtual void printArrays() = 0;

	virtual void print() {
		for (int i = 1; i< input_max; ++i) {
			printf("\t%d",i);
		}
		printf("\n");
		for (int i = 1; i< getStateSum(); ++i) {
			printf("s%d",i);
			int l;
			if ((l = isStopState(i)) != 0) printf("#%d",l);
			for (int j = 1; j< input_max; ++j) {
				if (nextState(i,j) != 0)
					printf("\ts%d",nextState(i,j));
				else
					printf("\t");
			}
			printf("\n");
		}
	}
};

}  // namespace dragontooth