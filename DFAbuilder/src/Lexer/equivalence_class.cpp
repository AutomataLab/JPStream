#include "Lexer/equivalence_class.hpp"

using namespace std;

namespace dragontooth {

EquivalenceClass::EquivalenceClass() : vec(256, 1) {}

EquivalenceClass::~EquivalenceClass() {}

void EquivalenceClass::Add(regex_char ch) { insert(ch); }

void EquivalenceClass::Add(const std::vector<regex_char>& str) {
    for (auto p : str) insert(p);
}

void EquivalenceClass::Add(CharSet& cset) {
    auto charset = cset.charset;
    std::map<regex_char, regex_char> m;
    regex_char t;
    for (auto i = charset.begin(); i != charset.end(); ++i) {
        if (i->second.type == 0) {
            insert(i->first, m);
        } else if (i->second.type == 1) {
            t = i->first;
            ++i;
            i->first;
            insert(t, i->first, m);
        }
    }
}

void EquivalenceClass::Rearrage() {
    vector<regex_char> v(eclass_sum + 1);

    for (auto i : vec) if (i != 0) v[i] = 1;
    int p = 0;
    for (int i = 1; i <= eclass_sum; ++i)
        if (v[i] != 0) v[i] = ++p;

    eclass_sum = p;
    for (auto& i : vec) i = v[i];
    vec[0] = 0;
}

void EquivalenceClass::insert(regex_char k) { vec[k] = ++eclass_sum; }

void EquivalenceClass::insert(regex_char k,
                              std::map<regex_char, regex_char>& m) {
    regex_char key = vec[k];
    regex_char value;
    auto p = m.find(key);
    if (p == m.end()) {
        regex_char newec = ++eclass_sum;
        m[key] = newec;
        value = newec;
    } else {
        value = p->second;
    }
    vec[k] = value;
}

void EquivalenceClass::insert(regex_char p, regex_char q,
                              std::map<regex_char, regex_char>& m) {
    regex_char cache_key = -1, cache_value, key, value;
    for (regex_char k = p; k <= q; ++k) {
        key = vec[k];
        if (key == cache_key)
            value = cache_value;
        else {
            auto p = m.find(key);
            if (p == m.end()) {
                regex_char newec = ++eclass_sum;
                m[key] = newec;
                value = newec;
            } else {
                value = p->second;
            }
            cache_key = key;
            cache_value = value;
        }
        vec[k] = value;
    }
}

regex_char EquivalenceClass::makeChar(regex_char ch) {
    return getClass(ch);
}

std::vector<regex_char> EquivalenceClass::makeString(const std::vector<regex_char>& str) {
    std::vector<regex_char> result;
    for (auto ch : str) {
        result.push_back(getClass(ch));
    }
    return result;
}

set<regex_char> EquivalenceClass::makeSet(CharSet& charset) {
    set<regex_char> class_set;
    regex_char t, last;
    if (charset.negate)
        for (regex_char i = 1; i <= eclass_sum; ++i) 
            class_set.insert(i);

    for (auto i = charset.charset.begin(); i != charset.charset.end(); ++i) {
        if (i->second.type == 0) {
            if (!charset.negate) 
                class_set.insert(vec[i->first]);
            else
                class_set.erase(vec[i->first]);
        }
        if (i->second.type == 1) {
            t = i->first;
            ++i;
            last = 0;
            for (regex_char k = t; k <= i->first; ++k) {
                if (last != vec[k]) {
                    last = vec[k];
                    if (!charset.negate)
                        class_set.insert(last);
                    else 
                        class_set.erase(last);
                }
            }
        }
    }
    return class_set;
}

regex_char EquivalenceClass::getClass(regex_char c) {
    if (c >= vec.size()) return 0;
    return vec[c];
}

}  // namespace dragontooth