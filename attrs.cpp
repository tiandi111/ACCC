//
// Created by 田地 on 2021/6/27.
//

#include <string>
#include <unordered_set>

#include "attrs.h"

using namespace std;
using namespace cool;
using namespace attr;

shared_ptr<TypeAttr> TypeAttr::LeastCommonAncestor(vector<shared_ptr<TypeAttr>>& attrs) {
    shared_ptr<TypeAttr> typeAttr = attrs.front();
    for (auto& attr : attrs) {
        typeAttr = typeAttr->LeastCommonAncestor(attr);
    }
    return typeAttr;
}

shared_ptr<TypeAttr> TypeAttr::LeastCommonAncestor(const shared_ptr<TypeAttr>& another) const {
    unordered_set<string> ancs = {name};
    auto cur = parent;
    while (cur) {
        ancs.insert(cur->name);
        cur = cur->parent;
    }
    cur = another;
    while (cur) {
        if (ancs.find(cur->name) != ancs.end()) return cur;
        cur = cur->parent;
    }
    throw runtime_error("no common ancestor found");
}

bool TypeAttr::Conforms(const string& type) const {
    if (name == type) return true;
    auto ptr = parent;
    while (ptr && ptr->name != type) {
        ptr = ptr->parent;
    }
    return ptr && ptr->name == type;
}

bool TypeAttr::Equal(const string& type) const {
    return name == type;
}

bool TypeAttr::Equal(const TypeAttr& type) const {
    return name == type.name;
}