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

namespace cool {

namespace type {

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
    TypeAdvisor(const string& rootType);

    bool Contains(const string& type);

    void AddType(shared_ptr<repr::Class> cls);

    shared_ptr<repr::Class> GetTypeRepr(const string& type);

    bool Conforms(const string& child, const string& parent);

    string LeastCommonAncestor(const string& typea, const string& typeb);

    string LeastCommonAncestor(vector<string>& types);

    void BottomUpVisit(const string& type, function<bool(shared_ptr<repr::Class>)> f);
};

} // namespace type

} // namespace cool

#endif //COOL_TYPEAD_H
