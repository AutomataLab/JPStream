#pragma once

#include "Lexer/equivalence_class.hpp"
#include "pass.hpp"

namespace dragontooth {

/**
 * @brief This class is using the equivalence class to convert set string to
 * actual charset
 */
class ECSetConverter : public Pass {
public:
    ECSetConverter() { ec = new EquivalenceClass(); }
    virtual ~ECSetConverter() {
        if (ec) {
            delete ec;
            ec = nullptr;
        }
    }

    virtual void Prescan(RegexItem* root);
    virtual void Convert(RegexItem* root);

    // We need to prescan all data in this model including string and char to
    // get all possible char that move together as equivalence class
    virtual void PrescanAll(RegexItem* root) { traverses_model(root, true); }

    // After prescan building all the equivalence class, we need remove the useless part
    virtual void Rearrage() { ec->Rearrage(); }
    
    // Then we can convert all data (not only set) into equivalence class id
    virtual void ConvertAll(RegexItem* root) { traverses_model(root); }

    virtual IPassable* Execute(IPassable* data, IPassable* join_data);
    virtual IPassable* Branch(IPassable* data) { return ec; }
    
    int size() { return ec->getSum(); }
    void print();
    
protected:
    // for each node in this tree, if it's a set, then run convert
    void traverses_model(RegexItem* root, bool isPrescan = false);
    EquivalenceClass* ec;
};
}  // namespace dragontooth