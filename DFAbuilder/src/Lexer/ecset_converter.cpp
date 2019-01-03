#include "Lexer/ecset_converter.hpp"
#include "Lexer/charset.hpp"
#include "Lexer/equivalence_class.hpp"
#include "Lexer/model_core.hpp"
#include "Lexer/regex_model.hpp"

#include <iostream>

using namespace std;

namespace dragontooth {

void ECSetConverter::Prescan(RegexItem* root) {
    switch (root->getItemType()) {
        case RegexItem::rt_set: {
            RegexSet* ptr = dynamic_cast<RegexSet*>(root);
            CharSet cset(ptr->getData());
            ec->Add(cset);
            return;
        }
        case RegexItem::rt_char: {
            // FIXME: Sometimes the ecset builder is not right
            RegexChar* ptr = dynamic_cast<RegexChar*>(root);
            ec->Add(ptr->getChar());
            return;
        }
    }
}

void ECSetConverter::Convert(RegexItem* root) {
    switch (root->getItemType()) {
        case RegexItem::rt_set: {
            RegexSet* ptr = dynamic_cast<RegexSet*>(root);
            CharSet cset(ptr->getData());
            ptr->setCharset(ec->makeSet(cset));
            return;
        }
        case RegexItem::rt_char: {
            RegexChar* ptr = dynamic_cast<RegexChar*>(root);
            ptr->setChar(ec->makeChar(ptr->getChar()));
            return;
        }
    }
}

IPassable* ECSetConverter::Execute(IPassable* data, IPassable* join_data) {
    RegexModel* model = dynamic_cast<RegexModel*>(data);
    for (int i = 0; i < model->size(); ++i) {
        auto p = model->at(i);
        PrescanAll(p->root);
    }
    Rearrage();
    for (int i = 0; i < model->size(); ++i) {
        auto p = model->at(i);
        ConvertAll(p->root);
    }
    model->input_max = ec->getSum();
    model->ec = ec;
    return model;
}

void ECSetConverter::traverses_model(RegexItem* root, bool isPrescan) {
    switch (root->getItemType()) {
        case RegexItem::rt_list: {
            RegexList* ptr = dynamic_cast<RegexList*>(root);
            auto& item = ptr->getItems();
            for (auto p : item) {
                traverses_model(p, isPrescan);
            }
            break;
        }
        case RegexItem::rt_ref: {
            RegexRef* ptr = dynamic_cast<RegexRef*>(root);
            traverses_model(ptr->getRef(), isPrescan);
            break;
        }
        default:
            if (isPrescan)
                Prescan(root);
            else
                Convert(root);
    }
}

void ECSetConverter::print() {
    for (int i = 0; i < ec->getInputMax(); ++i) {
        // cout << ec->getClass(i);
        if (ec->getClass(i) != 0)
            cout << char(i) << " -> " << ec->getClass(i) << endl;
    }
    cout << endl;
}

}  // namespace dragontooth