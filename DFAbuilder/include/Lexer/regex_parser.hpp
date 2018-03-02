/**
 * Regex parser is used for parsing regex in configuration
 */

#pragma once

#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "Lexer/model_core.hpp"
#include "Lexer/regex_model.hpp"
#include "pass.hpp"

namespace dragontooth {
/**
 * @brief RegexParser is used for parsing regex in configuration
 *
 * RegexParser is used to parse Regex in token definition.
 * Input: BNFModel*
 * Output: RegexItemList*
 */
class RegexParser : public Pass {
public:
    RegexParser() {}
    virtual ~RegexParser() {}

    /**
     * @brief The main function in RegexParser, used to analysis the
     * configuration
     * @return if the analysis works, it will set the model field and return it.
     * 		   if not, it will return nullptr.
     */
    RegexItem* Analysis(const std::string& data);

    virtual IPassable* Execute(IPassable* data, IPassable* join_data);

    RegexItem* model = nullptr;
    RegexRefMapper* mapper = nullptr;

protected:
    std::set<std::string> constants;
};

}  // namespace dragontooth