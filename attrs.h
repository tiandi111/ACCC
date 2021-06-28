//
// Created by 田地 on 2021/6/17.
//

#ifndef COOL_ATTRS_H
#define COOL_ATTRS_H

#include <string>
#include <vector>

using namespace std;

namespace cool {

namespace attr {

struct Attr {};

struct FuncAttr {
    string name;
    string type;
};

struct IdAttr {
    string name;
    string type;
};

struct TypeAttr {
    string name;
    shared_ptr<TypeAttr> parent;
    vector<shared_ptr<TypeAttr>> children;

    shared_ptr<TypeAttr> LeastCommonAncestor(const shared_ptr<TypeAttr>& another) const;

    bool Conforms(const string& type) const;

    bool Equal(const string& type) const;

    bool Equal(const TypeAttr& type) const;

    static shared_ptr<TypeAttr> LeastCommonAncestor(vector<shared_ptr<TypeAttr>>& attrs);
};

} // attr

} // cool

#endif //COOL_ATTRS_H
