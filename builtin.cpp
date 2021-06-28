//
// Created by 田地 on 2021/6/23.
//

#include <memory>

#include "builtin.h"
#include "repr.h"

using namespace std;
using namespace cool;
using namespace builtin;

repr::Class builtin::Object = {
    "Object",
    "",
    {
        make_shared<repr::FuncFeature>(Abort),
        make_shared<repr::FuncFeature>(TypeName),
        make_shared<repr::FuncFeature>(Copy),
    },
    {},
};

repr::FuncFeature builtin::Abort = {
    "abort",
    "Object"
};

repr::FuncFeature builtin::TypeName = {
    "type_name",
    "String"
};

repr::FuncFeature builtin::Copy = {
    "copy",
    "SELF_TYPE"
};

repr::Class builtin::IO = {
    "IO",
    "Object",
    {
        make_shared<repr::FuncFeature>(OutString),
        make_shared<repr::FuncFeature>(OutInt),
        make_shared<repr::FuncFeature>(InString),
        make_shared<repr::FuncFeature>(InInt),
    },
    {},
};

repr::FuncFeature builtin::OutString = {
    "out_string",
    "SELF_TYPE",
    nullptr,
    {make_shared<repr::Formal>(repr::Formal{"x", "String"})}
};

repr::FuncFeature builtin::OutInt = {
    "out_int",
    "SELF_TYPE",
    nullptr,
    {make_shared<repr::Formal>(repr::Formal{"x", "Int"})}
};

repr::FuncFeature builtin::InString = {
    "in_string",
    "String",
};

repr::FuncFeature builtin::InInt = {
    "in_int",
    "Int",
};

repr::Class builtin::Int = {
    "Int",
    "Object"
};

repr::Class builtin::String = {
    "String",
    "Object",
    {
        make_shared<repr::FuncFeature>(Length),
        make_shared<repr::FuncFeature>(Concat),
        make_shared<repr::FuncFeature>(Substr),
    },
    {},
};

repr::FuncFeature builtin::Length = {
    "length",
    "Int",
};

repr::FuncFeature builtin::Concat = {
    "String",
    "String",
};

repr::FuncFeature builtin::Substr = {
    "substr",
    "String",
    nullptr,
    {
        make_shared<repr::Formal>(repr::Formal{"i", "Int"}),
        make_shared<repr::Formal>(repr::Formal{"l", "Int"})
    }
};

repr::Class builtin::Bool = {
    "Bool",
    "Object",
};

unordered_set<string> builtin::BuiltinClassSet = {
    "Object",
    "IO",
    "Int",
    "String",
    "Bool"
};