//
// Created by 田地 on 2021/6/28.
//

#include "typead.h"

using namespace cool;
using namespace type;

TypeAdvisor::TypeAdvisor(const string& rootType) {
    root = make_shared<Node>(Node{rootType});
    map.insert({rootType, root});
}

bool TypeAdvisor::Contains(const string& type) {
    return map.find(type) != map.end();
}

void TypeAdvisor::AddType(shared_ptr<repr::Class> cls) {
    auto type = cls->name;
    auto parent = cls->parent;
    if (map.find(type) != map.end()) {
        throw runtime_error("type already existed");
    }
    auto parentNode = root;
    if (!parent.empty()) {
        parentNode = map.at(parent);
    }
    auto node = make_shared<Node>(Node{type, cls, parentNode});
    parentNode->children.emplace_back(node);
    map.insert({type, node});
}

shared_ptr<repr::Class> TypeAdvisor::GetTypeRepr(const string& type) {
    if (map.find(type) == map.end()) return nullptr;
    return map.at(type)->cls;
}

bool TypeAdvisor::Conforms(const string& child, const string& parent) {
    auto node = map.at(child);
    while (node) {
        if (node->type == parent) return true;
        node = node->parent;
    }
    return false;
}

string TypeAdvisor::LeastCommonAncestor(const string& typea, const string& typeb) {
    if (typea == typeb) return typea;
    auto node = map.at(typea);
    unordered_set<string> acs;
    while (node) {
        acs.insert(node->type);
        node = node->parent;
    }
    node = map.at(typeb);
    while (node) {
        if (acs.find(node->type) != acs.end()) return node->type;
        node = node->parent;
    }
    return "";
}

string TypeAdvisor::LeastCommonAncestor(vector<string>& types) {
    auto lca = types.front();
    for (auto& t : types) {
        lca = LeastCommonAncestor(lca, t);
    }
    return lca;
}

void TypeAdvisor::BottomUpVisit(const string& type, function<bool(shared_ptr<repr::Class>)> f) {
    if (map.find(type) == map.end()) throw runtime_error("type not found");
    auto node = map.at(type);
    while (node) {
        bool stop = f(node->cls);
        if (stop) return;
        node = node->parent;
    }
}