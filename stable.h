//
// Created by 田地 on 2021/6/17.
//

#ifndef COOL_STABLE_H
#define COOL_STABLE_H

#include <utility>
#include <vector>
#include <stack>
#include <string>
#include <unordered_map>
#include <memory>

#include "attrs.h"

using namespace std;
using namespace cool;

#define NEW_SCOPE_GUARD(scope, stmt, ...)\
    scope.NewScope(__VA_ARGS__);\
    stmt;\
    scope.FinishScope();

#define ENTER_SCOPE_GUARD(scope, stmt)\
    scope.EnterScope();\
    stmt;\
    scope.LeaveScope();

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
        return val.at(stack.back());
    }
};

class SymbolTable {
  private:
    using IdAttrMap = unordered_map<string, shared_ptr<attr::IdAttr>>;

    unordered_map<string, shared_ptr<attr::IdAttr>> ids;
    shared_ptr<repr::Class> cls;

  public:

    SymbolTable() = default;

    SymbolTable(shared_ptr<repr::Class> _cls) {
        cls = move(_cls);
    }

    bool ContainsId(const string& name) {
        return ids.find(name) != ids.end();
    }

    shared_ptr<attr::IdAttr> GetIdAttr(const string& name)  {
        return ids.at(name);
    }

    const IdAttrMap & GetIdAttrMap() {
        return ids;
    }

    void Insert(attr::IdAttr attr) {
        ids[attr.name] = make_shared<attr::IdAttr>(attr);
    }

    shared_ptr<repr::Class> GetClass() {
        return cls;
    }
};

template<typename T>
class ScopedTableSpecializer : ScopedTable<T> {};

template<>
class ScopedTableSpecializer<SymbolTable> : public ScopedTable<SymbolTable> {
  public:
    void NewScope(shared_ptr<repr::Class> cls) {
        stack.push_back(next++);
        val.emplace_back(SymbolTable(move(cls)));
    }

    shared_ptr<attr::IdAttr> GetIdAttr(const string& name) {
        for (int i = stack.size()-1; i>=0; i--) {
            auto attr = val.at(stack.at(i)).GetIdAttr(name);
            if (!attr) return attr;
        }
        return nullptr;
    }

    void Insert(attr::IdAttr attr) {
        Current().Insert(attr);
    }

    shared_ptr<repr::Class> GetClass() {
        return Current().GetClass();
    }
};


#endif //COOL_STABLE_H
