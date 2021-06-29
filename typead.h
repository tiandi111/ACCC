//
// Created by 田地 on 2021/6/28.
//

#ifndef COOL_TYPEAD_H
#define COOL_TYPEAD_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>

#include "repr.h"

using namespace std;

class TypeAdvisor {
  private:
    struct Node {
        string type;
        shared_ptr<repr::Class> cls;
        shared_ptr<Node> parent;
        vector<shared_ptr<Node>> children;
    };
    shared_ptr<Node> root;
    unordered_map<string, shared_ptr<Node>> map;

  public:
    TypeAdvisor(const string& rootType) {
        root = make_shared<Node>(Node{rootType});
        map.insert({rootType, root});
    }

    bool Contains(const string& type) {
        return map.find(type) != map.end();
    }

    void AddType(shared_ptr<repr::Class> cls) {
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

    shared_ptr<repr::Class> GetTypeRepr(const string& type) {
        if (map.find(type) == map.end()) return nullptr;
        return map.at(type)->cls;
    }

    bool Conforms(const string& child, const string& parent) {
        auto node = map.at(child);
        while (node) {
            if (node->type == parent) return true;
            node = node->parent;
        }
        return false;
    }

    string LeastCommonAncestor(const string& typea, const string& typeb) {
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

    string LeastCommonAncestor(vector<string>& types) {
        auto lca = types.front();
        for (auto& t : types) {
            lca = LeastCommonAncestor(lca, t);
        }
        return lca;
    }

    void BottomUpVisit(const string& type, function<bool(shared_ptr<repr::Class>)> f) {
        if (map.find(type) == map.end()) throw runtime_error("type not found");
        auto node = map.at(type);
        while (node) {
            bool stop = f(node->cls);
            if (stop) return;
            node = node->parent;
        }
    }
};

#endif //COOL_TYPEAD_H
