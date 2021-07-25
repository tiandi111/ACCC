//
// Created by 田地 on 2021/6/17.
//

#ifndef COOL_ATTRS_H
#define COOL_ATTRS_H

#include <string>
#include <vector>
#include <memory>

#include "repr.h"

using namespace std;

namespace cool {

namespace attr {

struct Attr {};

struct FuncAttr {
    string name;
    string type;
};

struct IdAttr {
    enum StorageClass {
        Field,
        Local,
        Arg,
    };
    StorageClass storageClass;
    uint32_t idx;
    string name;
    string type;

    IdAttr(const string& _name, const string& _type)
    : name(_name), type(_type) {}
};

struct TypeAttr {
    string name;
    string parent;
    shared_ptr<repr::Class> cls;
    inline string Name() { return cls->name.val; }
    inline string Parent() { return cls->parent.val; }
    inline bool Is(const string& type) { return cls->name.val == type; }
};

} // attr

} // cool

#endif //COOL_ATTRS_H
