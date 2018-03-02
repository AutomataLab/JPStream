#include "Lexer/charset.hpp"
#include "Lexer/utility.hpp"

namespace dragontooth {

#include <vector>
using namespace std;

CharSet::CharSet() {}

CharSet::~CharSet() {}

CharSet::CharSet(const CharSet& copy) {
    eclass_sum = copy.eclass_sum;
    charset.insert(copy.charset.begin(), copy.charset.end());
}

CharSet::CharSet(const std::string& _str) {
    this->str = _str;
	regex_char last = 0;
	this->eclass_sum = 1;
	bool isConnector = false;
	for (auto i = str.begin(); i != str.end(); ++i)
	{
		if (i == str.begin() && (*i == '^')) {
			negate = true;
		}
		else {
			if (*i == '-') {
				if (last == 0 || isConnector){
					// throw exception("error, \'-\' is not right");
				}

				isConnector = true;
			}
			else {
				regex_char c = *i;
				if (c == '\\') {
					c = CharEscape(i);
				}

				if (isConnector) {
					insert(last,c);
					last = 0;
					isConnector = false;
				}
				else {
					if (last != 0) {
						insert(last);
						last = 0;
					}
					last = c;
				}
			}
		}
	}
	if (last != 0) {
		insert(last);
	}
}

void CharSet::insert(regex_char p, regex_char q) {
    if (p == q) {
        insert(p);
        return;
    }
    charset[p].type = 1;
    charset[q].type = -1;
    map<regex_char, charset_value>::iterator f, l;
    auto i = charset.find(p);
    if (i == charset.begin() ||
        ((i->first - (--i)->first > 1) && (i->second.type != 1))) {
        // remove from here, without itself
        if (i->first == p)
            f = ++i;
        else
            f = ++++i;  // - -!
    } else {
        if (i->second.type == 1) {
            // remove from here, withitself
            f = ++i;
        } else {
            // remove before here
            f = i;
        }
    }

    auto j = charset.find(q);
    if ((++j) == charset.end() ||
        (j->first - (--j)->first > 1) && ((++j)->second.type != -1)) {
        // remove before here
        l = charset.find(q);
    } else {
        if (j->second.type == -1) {
            // remove before here with itself
            l = j;
        } else {
            // remove after here
            l = ++j;
        }
    }
    if (l->first > f->first) charset.erase(l, f);
}

void CharSet::insert(regex_char c) {
    auto i = charset.find(c);
    if (i == charset.end()) {
        charset[c].type = 0;
        i = charset.find(c);
        auto j = i;
        if (j != charset.begin() && ((--j)->second.type == 1)) {
            charset.erase(i);
            return;
        }
        j = i;
        if (j != charset.begin() && ((--j)->second.type == -1) &&
            (i->first - j->first == 1)) {
            charset.erase(j);
            i->second.type = -1;
            return;
        }
        j = i;
        if ((++j) != charset.end() && (j->second.type == 1) &&
            (j->first - i->first == 1)) {
            charset.erase(j);
            i->second.type = 1;
            return;
        }
    }
}

bool operator==(const charset_value& q, const charset_value& p) {
    return ((q.type == p.type) && (q.eclass == p.eclass));
}

bool CharSet::operator==(const CharSet& p) {
    return (this->charset == p.charset);
}
}