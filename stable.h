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

#define NEW_SCOPE_GUARD(scope, stmt)\
    scope.NewScope();\
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

    // todo: fix: when there's no val in stack
    T& Current() {
        if (stack.empty()) {
            throw runtime_error("no scope to return, call NewScope or EnterScope first");
        }
        return val.at(stack.back());
    }
};

// todo: invoke methods recursively on scoped symbol tables
class SymbolTable {
  private:
    using FuncAttrMap = unordered_map<string, shared_ptr<attr::FuncAttr>>;
    using IdAttrMap = unordered_map<string, shared_ptr<attr::IdAttr>>;
    using TypeAttrMap = unordered_map<string, shared_ptr<attr::TypeAttr>>;

    unordered_map<string, shared_ptr<attr::FuncAttr>> funcs;
    unordered_map<string, shared_ptr<attr::IdAttr>> ids;
    unordered_map<string, shared_ptr<attr::TypeAttr>> types;
    shared_ptr<repr::Class> cls;

  public:

    SymbolTable() = default;

    bool ContainsFunc(const string& name) {
        return funcs.find(name) != funcs.end();
    }

    bool ContainsId(const string& name) {
        return ids.find(name) != ids.end();
    }

    bool ContainsType(const string& name) {
        return types.find(name) != types.end();
    }

    shared_ptr<attr::FuncAttr> GetFuncAttr(const string& name)   {
        return funcs.at(name);
    }

    shared_ptr<attr::IdAttr> GetIdAttr(const string& name)  {
        return ids.at(name);
    }

    shared_ptr<attr::TypeAttr> GetTypeAttr(const string& name)  {
        return types.at(name);
    }

    const FuncAttrMap& GetFuncAttrMap() {
        return funcs;
    }

    const IdAttrMap & GetIdAttrMap() {
        return ids;
    }

    const TypeAttrMap & GetTypeAttrMap() {
        return types;
    }

    void Insert(attr::FuncAttr attr) {
        funcs[attr.name] = make_shared<attr::FuncAttr>(attr);
    }

    void Insert(attr::IdAttr attr) {
        ids[attr.name] = make_shared<attr::IdAttr>(attr);
    }

    void Insert(attr::TypeAttr attr) {
        types[attr.name] = make_shared<attr::TypeAttr>(attr);
    }
};

#endif //COOL_STABLE_H
