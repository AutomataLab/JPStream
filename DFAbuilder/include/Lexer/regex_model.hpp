/**
 * Regex parser is used for parsing regex in configuration
 * This file is the output model of the Regex Parser. This part will also be
 * reused in DFA Creation.
 */

#pragma once

#include <stdint.h>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace dragontooth {

// The internal char definition
typedef uint32_t regex_char;

class RegexList;
class RegexSet;
class RegexString;
class RegexChar;
class RegexRef;

/**
 * @brief RegexItem is the basic class of the model. It has all the common
 * methods for all Items and it defined the Extern Operator of this Item.
 */
class RegexItem {
public:
    RegexItem() {}
    virtual ~RegexItem() {}

    enum RegexItemType { rt_list, rt_set, rt_string, rt_char, rt_ref };
    enum RegexExternOpt {
        re_none,
        re_optional,
        re_repetition,
        re_nonzero_repetition
    };
    virtual RegexItemType getItemType() const = 0;
    virtual RegexItem* deepCopy() const = 0;
    virtual void setOpt(RegexExternOpt opt) { this->opt = opt; }
    virtual RegexExternOpt getOpt() { return opt; }

protected:
    RegexExternOpt opt = re_none;

    friend std::ostream& operator<<(std::ostream& out, const RegexItem& that);

public:
    virtual void BuildAll();
    virtual void BuildNullable();
    virtual void BuildFirstAndLast();
    virtual void BuildFollow();
    virtual void addTerminator();
    virtual bool isEdge(regex_char) const { return false; }
    virtual void printFollowpos() {}
    bool isTerminator = false;
    bool nullable;
    std::set<RegexItem*> firstpos;
    std::set<RegexItem*> lastpos;
    std::set<RegexItem*> followpos;
};

/**
 * @brief It's the list of Items, if enableOr is ture, it means the relationship
 * between each Item is Or.
 */
class RegexList : public RegexItem {
public:
    RegexList(bool enableOr = false) { this->enableOr = enableOr; }
    RegexList(const RegexList& other) : enableOr(other.enableOr) {
        for (auto p : other.items) items.push_back(p->deepCopy());
    }
    virtual ~RegexList() {}

    void Add(RegexItem* item) { items.push_back(item); }
    virtual RegexItemType getItemType() const { return rt_list; }
    std::vector<RegexItem*>& getItems() { return items; }
    const std::vector<RegexItem*>& getItems() const { return items; }

    virtual RegexItem* deepCopy() const { return new RegexList(*this); }

protected:
    bool enableOr;
    std::vector<RegexItem*> items;

    virtual void BuildNullable();
    virtual void BuildFirstAndLast();
    virtual void BuildFollow();
    virtual void printFollowpos() {
        for (auto p : items) {
            p->printFollowpos();
            std::cout << std::endl;
        }
    }

    friend std::ostream& operator<<(std::ostream& out, const RegexList& that) {
        bool first = true;
        for (auto p : that.items) {
            if (first) {
                out << *p;
                first = false;
                continue;
            }
            if (that.enableOr)
                out << " | ";
            else
                out << ' ';
            out << *p;
        }
        if (that.opt == re_optional) out << '?';
        if (that.opt == re_repetition) out << '*';
        if (that.opt == re_nonzero_repetition) out << '+';
        return out;
    }
};

/**
 * The string in regex. It can effectively reduce the number of nodes.
 */
class RegexString : public RegexItem {
public:
    RegexString(const char* str) : data(str) { escapeString(); }
    RegexString(const RegexString& other)
        : data(other.data), ch_data(other.ch_data) {}
    virtual ~RegexString() {}
    virtual RegexItemType getItemType() const { return rt_string; }
    virtual RegexItem* deepCopy() const { return new RegexString(*this); }
    virtual bool isEdge(regex_char ch) const {
        if (ch_data.empty())
            return false;
        else
            return ch_data[0] == ch;
    }
    const std::vector<regex_char>& getChData() const { return ch_data; }
    std::vector<regex_char>& getChData() { return ch_data; }

protected:
    std::string data;
    std::vector<regex_char> ch_data;
    void escapeString();

    virtual void printFollowpos() {
        std::cout << "string: " << data << std::endl;
        for (auto p : followpos) {
            std::cout << '\t' << *p;
        }
    }

    friend std::ostream& operator<<(std::ostream& out,
                                    const RegexString& that) {
        out << '"' << that.data << '"';
        if (that.opt == re_optional) out << '?';
        if (that.opt == re_repetition) out << '*';
        if (that.opt == re_nonzero_repetition) out << '+';
        return out;
    }
};

/**
 * One char in regex
 */
class RegexChar : public RegexItem {
public:
    RegexChar() {}
    // create an escape character, if no escape, we just create the character
    RegexChar(const char* tr) { ch = escape_char(tr); }
    RegexChar(const RegexChar& other) : ch(other.ch) {}
    virtual ~RegexChar() {}
    virtual RegexItemType getItemType() const { return rt_char; }
    virtual RegexItem* deepCopy() const { return new RegexChar(*this); }
    virtual bool isEdge(regex_char ch) const { return this->ch == ch; }

    virtual void printFollowpos() {
        std::cout << "char: " << (char)ch << std::endl;
        for (auto p : followpos) {
            std::cout << '\t' << *p;
        }
    }

    regex_char getChar() const { return ch; }
    regex_char& getChar() { return ch; }

protected:
    regex_char escape_char(const char* tr);
    regex_char ch = 0;

    friend std::ostream& operator<<(std::ostream& out, const RegexChar& that) {
        out << (char)that.ch;
        if (that.opt == re_optional) out << '?';
        if (that.opt == re_repetition) out << '*';
        if (that.opt == re_nonzero_repetition) out << '+';
        return out;
    }
};

/**
 * @brief This is the most important thing - Set in regex.
 * This model will just log your definition of this set, it wouldn't be parsed.
 * To parse a set, please use the Charset.
 */
class RegexSet : public RegexItem {
public:
    RegexSet() {}
    RegexSet(const char* tr) { data = tr; }
    RegexSet(const RegexSet& other)
        : data(other.data), charset(other.charset) {}
    virtual ~RegexSet() {}

    // create an escape character collection
    static RegexSet* getPreset(const char* tr);

    virtual RegexItemType getItemType() const { return rt_set; }
    virtual RegexItem* deepCopy() const { return new RegexSet(*this); }

    void setCharset(const std::set<regex_char>& charset) {
        this->charset = charset;
    }
    std::string getData() { return data; }

    virtual bool isEdge(regex_char ch) const {
        return charset.find(ch) != charset.end();
    }

    virtual void printFollowpos() {
        std::cout << "set: " << data << std::endl;
        for (auto p : charset) {
            std::cout << " " << (char)p;
        }
        std::cout << std::endl;
        for (auto p : followpos) {
            std::cout << '\t' << *p;
        }
    }

protected:
    std::string data;
    std::set<regex_char> charset;
    friend std::ostream& operator<<(std::ostream& out, const RegexSet& that) {
        out << '[' << that.data << ']';
        if (that.opt == re_optional) out << '?';
        if (that.opt == re_repetition) out << '*';
        if (that.opt == re_nonzero_repetition) out << '+';
        return out;
    }
};

// To use this class, you need to create a RegexRefMapper to hold all of the
// name and reference of regex.
class RegexRef : public RegexItem {
public:
    virtual ~RegexRef() {}
    virtual RegexItemType getItemType() const { return rt_ref; }
    RegexRef(const RegexRef& other) : name(other.name) {
        ref = other.ref->deepCopy();
    }
    RegexItem* getRef() { return ref; }

private:
    friend class RegexRefMapper;
    RegexRef(const std::string& name, RegexItem* ref) {
        this->name = name;
        this->ref = ref->deepCopy();
    }
    virtual RegexItem* deepCopy() const { return new RegexRef(*this); }

protected:
    std::string name;
    RegexItem* ref;
    virtual void BuildNullable();
    virtual void BuildFirstAndLast();
    friend std::ostream& operator<<(std::ostream& out, const RegexRef& that) {
        out << '{';
        out << that.name << " : " << *that.ref;
        out << '}';
        if (that.opt == re_optional) out << '?';
        if (that.opt == re_repetition) out << '*';
        if (that.opt == re_nonzero_repetition) out << '+';
        return out;
    }
};

/**
 * RegexRefMapper is a class designed to be implemeneted by user. This is only
 * contain the basic logic to find a reference in regex. But acturally, you need
 * more work to build a substring of regex. eg. To parse the regex and run the
 * BuildAll() first.
 */
class RegexRefMapper {
public:
    RegexRefMapper() {}
    virtual ~RegexRefMapper() {
        for (auto pair : mapper) {
            delete pair.second;
        }
    }

    virtual void Define(const std::string& name, RegexItem* item) {
        mapper[name] = new RegexRef(name, item);
    }

    virtual RegexRef* Find(const std::string& name) {
        auto it = mapper.find(name);
        if (it == mapper.end())
            return NULL;
        else
            return it->second;
    }

protected:
    std::map<std::string, RegexRef*> mapper;
};


// You can print the RegexItem using this way
std::ostream& operator<<(std::ostream& out, const RegexItem& that);

}  // namespace dragontooth