//
// Created by 田地 on 2021/6/28.
//

#include <iostream>

#include "typead.h"

using namespace cool;
using namespace type;

shared_ptr<TypeAdvisor::Node> TypeAdvisor::get(const string& type) {
    if (map.find(type) == map.end()) return nullptr;
    return map.at(type);
}

TypeAdvisor::TypeAdvisor(shared_ptr<repr::Class> rootClass) {
    root = make_shared<Node>(Node{rootClass->name.val, rootClass});
    map.insert({rootClass->name.val, root});
}

bool TypeAdvisor::Contains(const string& type) {
    return (map.find(type) != map.end()) && map.at(type);
}

void TypeAdvisor::AddType(shared_ptr<repr::Class> cls) {
    auto type = cls->name;
    auto parent = cls->parent;
    if (type.Empty()) throw runtime_error("type name cannot be ''");
    if (parent.Empty()) throw runtime_error("parent type cannot be ''");
    if (Contains(type.val)) throw runtime_error("type already existed");
    auto parentNode = root;
    if (!parent.Empty()) {
        parentNode = get(parent.val);
        if (!parentNode) throw runtime_error("parent type '" + parent.val + "' not found");
    }
    auto node = make_shared<Node>(Node{type.val, cls, parentNode});
    parentNode->children.emplace_back(node);
    map.insert({type.val, node});
}

shared_ptr<repr::Class> TypeAdvisor::GetTypeRepr(const string& type) {
    if (!Contains(type)) return nullptr;
    return get(type)->cls;
}

bool TypeAdvisor::Conforms(const string& child, const string& parent) {
    auto node = get(child);
    while (node) {
        if (node->type == parent) return true;
        node = node->parent;
    }
    return false;
}

string TypeAdvisor::LeastCommonAncestor(const string& typea, const string& typeb) {
    if (typea == typeb) return typea;
    auto node = get(typea);
    unordered_set<string> acs;
    while (node) {
        acs.insert(node->type);
        node = node->parent;
    }
    node = get(typeb);
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
    auto node = get(type);
    while (node) {
        if (!node->cls) std::cout<< node->type << " has null cls ptr" <<endl;
        bool stop = f(node->cls);
        if (stop) return;
        node = node->parent;
    }
}