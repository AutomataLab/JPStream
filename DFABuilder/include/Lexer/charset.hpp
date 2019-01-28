#pragma once

#include <map>
#include "Lexer/regex_model.hpp"

namespace dragontooth {

/**
 * @brief one of the tag in the tree of range
 */
struct charset_value {
    char type;
    regex_char eclass;
    charset_value() {
        type = 0;
        eclass = 0;
    }
    charset_value(char _type, regex_char _eclass) {
        type = _type;
        eclass = _eclass;
    }
    charset_value(const charset_value& p) {
        this->type = p.type;
        this->eclass = p.eclass;
    }
};

/**
 * @brief CharSet is a set parser. It will analysis the string definition and create a set.
 */
class CharSet {
public:
    CharSet();
    ~CharSet();
    CharSet(const std::string& _str);
    // copy constructor
    CharSet(const CharSet& copy);

    bool operator==(const CharSet&);

    // all the char is saved in the balanced tree.
    std::map<regex_char, charset_value> charset;

    // insert method , type = 0 means a standalone char
    // type = 1 means the begin of a interval, type = 2 means the end of the
    // interval. void insert(regex_char p, regex_char q, unsigned short eclass);
    void insert(regex_char p, regex_char q);
    void insert(regex_char c);

    // make the set negate
    bool negate = false;
    std::string str;
    regex_char eclass_sum;
};

}  // namespace dragontooth