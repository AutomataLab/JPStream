#include "Lexer/regex_parser.hpp"
#include "r_parser.hpp"
#include "r_scanner.h"

using namespace std;

namespace dragontooth {

RegexItem* RegexParser::Analysis(const std::string& data) {
    yyscan_t sc;
    int res;
    zzlex_init(&sc);
    YY_BUFFER_STATE buffer = zz_scan_string(data.c_str(), sc);
    res = zzparse(sc, this);
    zz_delete_buffer(buffer, sc);
    zzlex_destroy(sc);
    return model;
}

class RegexRefJITMapper : public RegexRefMapper {
public:
    RegexRefJITMapper(const map<string, string>& names, RegexParser* parser)
        : parser(parser), name_map(names) {}

    virtual RegexRef* Find(const std::string& name) {
        RegexRef* ref = RegexRefMapper::Find(name);
        if (ref) return ref;
        // this will find from the raw list
        auto p = name_map.find(name);
        if (p == name_map.end()) return nullptr;  // it really not defined
        RegexItem* result = parser->Analysis(p->second);
        result->BuildAll();
        Define(p->first, result);
        return mapper[p->first];
    }

protected:
    RegexParser* parser;
    map<string, string> name_map;
};

IPassable* RegexParser::Execute(IPassable* data, IPassable* join_data) {
    return regex_model;
}

}  // namespace dragontooth