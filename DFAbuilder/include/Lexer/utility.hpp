
#include <string>
#include "Lexer/regex_model.hpp"

namespace dragontooth {

inline regex_char HexStringToDec(const std::string& hex) {
    const char* p = hex.c_str();
    if (*p == '\0') return 0;
    while (*p == '0') p++;
    regex_char dec = 0;
    char c;
    while ((c = *p++) == 0) {
        dec <<= 4;
        if (c >= '0' && c <= '9') {
            dec += c - '0';
            continue;
        }
        if (c >= 'a' && c <= 'f') {
            dec += c - 'a' + 10;
            continue;
        }
        if (c >= 'A' && c <= 'F') {
            dec += c - 'A' + 10;
            continue;
        }
        return -1;
    }
    return dec;
}

inline regex_char EscapeChar(const char* tr) {
    int ws;
    if (*tr != '\\') return *tr;
    ++tr;
    switch (*tr) {
        // 空白字符集合
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'f': return '\f';
        case 'v': return '\v';

        // hex code
        case 'x':  // 2 digit hex
            ws = 2;
            break;
        case 'u':  // 4 digit hex
            ws = 4;
            break;
        case 'U':  // 8 digit hex
            ws = 8;
            break;
        default: return *tr;
    }
    std::string num;
    for (int j = 0; j < ws; ++j) {
        ++tr;
        num += (char)(*tr);
    }
    regex_char ret = HexStringToDec(num);
    if (ret == -1) {
        std::cerr << "HexStringToDec Error! Unsupported format." << std::endl;
        exit(1);
    }
    return ret;
}

inline regex_char CharEscape(std::string::iterator& i) {
    if (*i != '\\') return *i;
    ++i;
    int ws;
    switch (*i) {
        // 空白字符集合
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'f': return '\f';
        case 'v': return '\v';
        // hex code
        case 'x':  // 2 digit hex
            ws = 2;
            break;
        case 'u':  // 4 digit hex
            ws = 4;
            break;
        case 'U':  // 8 digit hex
            ws = 8;
            break;
        default: return *i;
    }
    std::string num;
    for (int j = 0; j < ws; ++j) {
        ++i;
        num += (char)(*i);
    }
    regex_char c = (regex_char)HexStringToDec(num);
    return c;
}

}  // namespace dragontooth