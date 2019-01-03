#include "Lexer/dfa_compressed.hpp"
#include "limits.h"

using namespace std;
namespace dragontooth {

int DFACompressed::nextState(int s, regex_char ch) {
    if (s == 0 || ch == 0) return 0;
    int k = m_base[s] + (int)ch;
    if ((k >= 0) && (k < Bottom) && (m_check[k] == s))
        return m_next[k];
    else {
        if (m_default[s] > 0) return nextState(m_default[s], ch);
        return -m_default[s];
    }
}

void DFACompressed::addEdge(int s, const std::vector<int>& objvec, int max) {
    // cout << "addEdge: " << s << endl;
    // for (auto p : objvec) {
    //     cout << p << ' ';
    // }
    // cout << endl;

    if (m_default.size() <= max) { 
        m_default.resize(max+1, 0);
        m_base.resize(max+1, 0);
        stopState.resize(max+1, 0);
    }
    int most_frq;
    int size = getValidSize(objvec, max, most_frq);
    if (size <= 10) {
        createNewArea(most_frq, s, objvec);
    } else {
        for (int i = 1; i < s; ++i) {
            bool ret = compareState(i, objvec);
            if (ret) { createLinkArea(i, s, objvec); return; }
        }
        createNewArea(most_frq, s, objvec);
    }
}

void DFACompressed::createNewArea(int base, int s, const std::vector<int>& objvec) {
    m_default[s] = -base;
    for (int i = Top; i <= Bottom; ++i) {
        // first check the space 
        bool is_first = true; 
        int offset; int last_valid = -1;
        for (int a = 1; a < objvec.size(); ++a) 
            if (base != objvec[a]) {
                if (is_first) {
                    is_first = false;
                    offset = i-a;
                }     
                last_valid = a;
            }
        if (last_valid == -1) { 
            m_base[s] = INT_MIN;
            return;
        }
        checkSpace(offset + last_valid + 1);

        // for each element valid in vector has a place
        bool b = true; 
        for (int a = 1; a < objvec.size(); ++a) {
            if (base != objvec[a]) {
                if (m_check[offset+a] != 0) {
                    b = false;
                    break;
                }
            }
        }
        if (b) {
            for (int a = 1; a < objvec.size(); ++a) 
                if (base != objvec[a]) {
                    m_next[offset+a] = objvec[a];
                    m_check[offset+a] = s;
                }
            m_base[s] = offset;
            if (Bottom < offset + last_valid + 1)
                Bottom = offset + last_valid + 1; 
            while (Top < Bottom && m_check[Top] != 0) Top++; // i + objvec.size() is empty, so it will stop
            return;
        }
    }
}

void DFACompressed::createLinkArea(int base, int s, const std::vector<int>& objvec) {
    m_default[s] = base;
    for (int i = Top; i <= Bottom; ++i) {
        // first check the space 
        bool is_first = true; 
        int offset; int last_valid = -1;
        for (int a = 1; a < objvec.size(); ++a) 
            if (nextState(base, a) != objvec[a]) {
                if (is_first) {
                    is_first = false;
                    offset = i-a;
                }     
                last_valid = a;
            }
        if (last_valid == -1) { 
            m_base[s] = INT_MIN;
            return;
        }
        checkSpace(offset + last_valid + 1);
        
        // for each element valid in vector has a place
        bool b = true;
        for (int a = 1; a < objvec.size(); ++a) {
            if (nextState(base, a) != objvec[a]) {
                if (m_check[offset+a] != 0) {
                    b = false;
                    break;
                }
            }
        }
        if (b) {
            for (int a = 1; a < objvec.size(); ++a) 
                if (nextState(base, a) != objvec[a]) {
                    m_next[offset+a] = objvec[a];
                    m_check[offset+a] = s;
                }
            m_base[s] = offset;
            if (Bottom < offset + last_valid + 1)
                Bottom = offset + last_valid + 1; 
            while (Top < Bottom && m_check[Top] != 0) Top++; // i + objvec.size() is empty, so it will stop
            return;
        }
    }
}

void DFACompressed::printArrays() {
    cout << "Top: " << Top << endl;
    cout << "Bottom: " << Bottom << endl;

    cout << "m_default:" << m_default.size() <<  endl;
    for (auto p : m_default) { cout << p << ' '; }
    cout << endl;
    cout << "m_base:" << m_base.size() << endl;
    for (auto p : m_base) { cout << p << ' '; }
    cout << endl;
    cout << "m_next:" << m_next.size() << endl;
    for (auto p : m_next) { cout << p << ' '; }
    cout << endl;
    cout << "m_check:" << m_check.size() << endl;
    for (auto p : m_check) { cout << p << ' '; }
    cout << endl;
}

}  // namespace dragontooth