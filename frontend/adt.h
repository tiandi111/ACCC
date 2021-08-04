//
// Created by 田地 on 2021/6/17.
//

#ifndef COOL_ADT_H
#define COOL_ADT_H

#include <utility>
#include <vector>
#include <stack>
#include <string>
#include <unordered_map>
#include <memory>

#include "attrs.h"

using namespace std;
using namespace cool;

namespace cool {

namespace adt {

#define NEW_SCOPE_GUARD(stable, stmt, ...)\
    stable.NewScope(__VA_ARGS__);\
    stmt;\
    stable.FinishScope();

#define ENTER_SCOPE_GUARD(stable, stmt)\
    stable.EnterScope();\
    stmt;\
    stable.LeaveScope();

template<typename T>
class ScopedTable {
  public:
    vector<int> stack;
    int next = 0;
    vector<T> val;

    ScopedTable() {
        InitTraverse();
    }

    void InitTraverse() {
        stack.clear();
        next = 0;
    }

    int Size() { return val.size(); }

    int Idx() {
        if (stack.empty())
            throw runtime_error("uninitialized ScopedTable, call InitTraverse() first");
        return stack.back();
    }

    vector<int>& Stack() { return stack; }

    void NewScope() {
        stack.push_back(next++);
        val.emplace_back(T());
    }

    void FinishScope() {
        if (stack.empty()) {
            throw runtime_error("no scope to finish");
        }
        stack.pop_back();
    }

    void EnterScope() {
        stack.push_back(next++);
        if (next > val.size()) {
            throw runtime_error("no more scopes to enter");
        }
    }

    void LeaveScope() {
        if (stack.empty()) {
            throw runtime_error("no scope to leave");
        }
        stack.pop_back();
    }

    T& Current() {
        if (stack.empty()) {
            throw runtime_error("no scope to return, call NewScope or EnterScope first");
        }
        return val.at(Idx());
    }
};

class SymbolTable {
  private:
    using IdAttrMap = unordered_map<string, shared_ptr<attr::IdAttr>>;

    int idx;
    int numLocalId;
    unordered_map<string, shared_ptr<attr::IdAttr>> ids;
    repr::Class* cls;

  public:
    SymbolTable() = default;

    SymbolTable(uint32_t _idx, repr::Class* _cls)
        : idx(_idx), cls(_cls) {}

    int NumLocalId() { return numLocalId; }

    int NextLocalIdIdx() { return numLocalId; }

    bool ContainsId(const string& name) {
        return ids.find(name) != ids.end();
    }

    shared_ptr<attr::IdAttr> GetIdAttr(const string& name)  {
        if (!ContainsId(name)) return nullptr;
        return ids.at(name);
    }

    const IdAttrMap & GetIdAttrMap() {
        return ids;
    }

    void Insert(attr::IdAttr attr) {
        if (attr.storageClass == attr::IdAttr::Local) numLocalId++;
        ids.insert({attr.name, make_shared<attr::IdAttr>(attr)});
    }

    repr::Class* GetClass() {
        return cls;
    }
};

template<typename T>
class ScopedTableSpecializer : ScopedTable<T> {};

template<>
class ScopedTableSpecializer<SymbolTable> : public ScopedTable<SymbolTable> {
public:
    void NewScope(repr::Class* cls) {
        val.emplace_back(SymbolTable(next, move(cls)));
        stack.push_back(next++);
    }

    shared_ptr<attr::IdAttr> GetIdAttr(const string& name) {
        for (int i = stack.size()-1; i>=0; i--) {
            auto attr = val.at(stack.at(i)).GetIdAttr(name);
            if (attr) return attr;
        }
        return nullptr;
    }

    void Insert(attr::IdAttr attr) {
        Current().Insert(attr);
    }

    repr::Class* GetClass() {
        return Current().GetClass();
    }

};

} // namespace adt

} // namespace cool

#endif //COOL_ADT_H
