#include "Lexer/regex_model.hpp"
#include "Lexer/charset.hpp"
#include "Lexer/utility.hpp"
using namespace std;
namespace dragontooth {

std::ostream& operator<<(std::ostream& out, const RegexItem& that) {
    switch (that.getItemType()) {
        case RegexItem::rt_list:
            out << dynamic_cast<const RegexList&>(that);
            break;
        case RegexItem::rt_char:
            out << dynamic_cast<const RegexChar&>(that);
            break;
        case RegexItem::rt_set:
            out << dynamic_cast<const RegexSet&>(that);
            break;
        case RegexItem::rt_ref:
            out << dynamic_cast<const RegexRef&>(that);
            break;
    }
    return out;
}

void RegexList::escapeString(const char* data) {
    for (auto p = data; *p != 0; ++p) {
        Add(new RegexChar(&p));
    }
}

regex_char RegexChar::escape_char(const char*& tr) {
    return EscapeChar(tr);
}

RegexSet* RegexSet::getPreset(const char* tr) {
    if (*tr == '.') return new RegexSet("^");
    ++tr;
    switch (*tr) {
        case 'd': return new RegexSet("0-9");
        case 'D': return new RegexSet("^0-9");
        case 'w': return new RegexSet("A-Za-z_");
        case 'W': return new RegexSet("^A-Za-z_");
        case 's': return new RegexSet(" \\t\\v\\f\\r\\n");
        case 'S': return new RegexSet("^ \\t\\v\\f\\r\\n");
    }
    return nullptr;
}

void RegexItem::BuildAll() {
    BuildNullable();
    BuildFirstAndLast();
    BuildFollow();
}

void RegexItem::addTerminator() {
    // Connect the last node to final
    for (auto p : lastpos) {
        p->isTerminator = true;
    }
}

void RegexItem::BuildNullable() {
    if (opt == re_optional || opt == re_repetition) nullable = true;
    else nullable = false;
}

void RegexRef::BuildNullable() {
    ref->BuildNullable();
    nullable = ref->nullable;
    if (opt == re_optional || opt == re_repetition) nullable = true;
}

void RegexList::BuildNullable() {
    for (auto p : items) 
        p->BuildNullable();

    RegexItem::BuildNullable();
    if (nullable == false) {
        if (enableOr) {
            for (auto p : items) {
                if (p->nullable) { nullable = true; return; }
            }
        } else {
            nullable = true;
            for (auto p : items) {
                if (!p->nullable) { nullable = false; return; }
            }
        }
    }
}

void RegexItem::BuildFirstAndLast() {
    firstpos.insert(this);
    lastpos.insert(this);
}

void RegexRef::BuildFirstAndLast() {
    ref->BuildFirstAndLast();
    firstpos = ref->firstpos;
    lastpos = ref->lastpos;
}

void RegexList::BuildFirstAndLast() {
    for (auto p : items) 
        p->BuildFirstAndLast();

    if (enableOr) {
        for (auto p : items) {
            for (auto q : p->firstpos)
                firstpos.insert(q);
            for (auto q : p->lastpos)
                lastpos.insert(q);
        }
    } else {
        for (auto i = items.begin(); i != items.end(); ++i) {
            for (auto q : (*i)->firstpos)
                firstpos.insert(q);
            if ((*i)->nullable == false) break;
        }
        for (auto i = items.rbegin(); i != items.rend(); ++i) {
            for (auto q : (*i)->lastpos)
                lastpos.insert(q);
            if ((*i)->nullable == false) break;
        }
    }
}

void RegexItem::BuildFollow() {
    if (opt == re_nonzero_repetition || opt == re_repetition) 
        for (auto q : lastpos) 
            for (auto k : firstpos)
                q->followpos.insert(k);
}

void RegexRef::BuildFollow() {
    ref->BuildFollow();
    RegexItem::BuildFollow();
}

void RegexList::BuildFollow() {
    for (auto p : items) 
        p->BuildFollow();

    RegexItem::BuildFollow();
    if (!enableOr) {
        RegexItem* last = nullptr;
        for (auto p = items.begin(); p != items.end(); ++p) {
            for (auto q = p+1; q != items.end(); ++q) {
                // for each pos in the firstpos set of p should be a follower of each lastpos set of last element
                for (auto t : (*p)->lastpos) {
                    for (auto k : (*q)->firstpos)
                        t->followpos.insert(k);
                }
                if ((*q)->nullable == false) break;
            }

            // here we deal with the non-greedy matching
            if ((*p)->nongreedy) {
                // printf("nongreedy\n");
                for (auto g : lastpos)
                    g->keepout = true;
            } 

        }
    }
}


}  // namespace dragontooth