#include "Lexer/set_converter.hpp"
#include "Lexer/charset.hpp"
#include "Lexer/model_core.hpp"
#include "Lexer/regex_model.hpp"

namespace dragontooth {

void SetConverter::Convert(RegexSet* itemset) {
    std::vector<bool> cvs; cvs.resize(256);
    CharSet cset(itemset->getData());
    if (cset.negate) {
        for (regex_char i = 0; i < 256; ++i) cvs[i] = true;
    }
    auto charset = cset.charset;
    regex_char t;
    for (auto i = charset.begin(); i != charset.end(); ++i) {
        if (i->second.type == 0) {
            insert(i->first, cset.negate, cvs);
        } else if (i->second.type == 1) {
            t = i->first;
            ++i;
            i->first;
            insert(t, i->first, cset.negate, cvs);
        }
    }
    std::set<regex_char> finalset;
    for (regex_char i = 0; i < 256; ++i) {
        if (cvs[i]) finalset.insert(i);
    }
    itemset->setCharset(finalset);
}

IPassable* SetConverter::Execute(IPassable* data, IPassable* join_data) {
    RegexModel* model = dynamic_cast<RegexModel*>(data);
    for (int i = 0; i < model->size(); ++i) {
        auto p = model->at(i);
        traverses_model(p->root);
    }
    return data;
}

void SetConverter::traverses_model(RegexItem* root) {
    switch (root->getItemType()) {
        case RegexItem::rt_list: {
            RegexList* ptr = dynamic_cast<RegexList*>(root);
            auto& item = ptr->getItems();
            for (auto p : item) {
                traverses_model(p);
            }
            break;
        }
        case RegexItem::rt_ref: {
            RegexRef* ptr = dynamic_cast<RegexRef*>(root);
            traverses_model(ptr->getRef());
            break;
        }
        case RegexItem::rt_set: {
            RegexSet* ptr = dynamic_cast<RegexSet*>(root);
            Convert(ptr);
            break;
        }
    }
}

void SetConverter::insert(regex_char k, bool negate, std::vector<bool>& cvs) {
    cvs[k] = !negate;
}

void SetConverter::insert(regex_char p, regex_char q, bool negate, std::vector<bool>& cvs) {
    for (regex_char i = p; i <= q; ++i) 
        cvs[i] = !negate;
}


}
