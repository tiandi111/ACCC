//
// Created by 田地 on 2021/6/23.
//

#include <memory>

#include "builtin.h"
#include "repr.h"

using namespace std;
using namespace cool;
using namespace builtin;
using namespace repr;

FuncFeature builtin::Abort = {
    StringAttr("abort"),
    StringAttr("Object"),
    make_shared<LinkBuiltin>(LinkBuiltin("Object")),
    {}
};

FuncFeature builtin::TypeName = {
    StringAttr("type_name"),
    StringAttr("String"),
    make_shared<LinkBuiltin>(LinkBuiltin("String")),
    {}
};

FuncFeature builtin::Copy = {
    StringAttr("copy"),
    StringAttr("SELF_TYPE"),
    make_shared<LinkBuiltin>(LinkBuiltin("SELF_TYPE")),
    {}
};

Class builtin::Object = {
   StringAttr("Object"),
   StringAttr(""),
   {
        make_shared<FuncFeature>(Abort),
        make_shared<FuncFeature>(TypeName),
        make_shared<FuncFeature>(Copy),
    },
    {},
};

FuncFeature builtin::OutString = {
    StringAttr("out_string"),
    StringAttr("SELF_TYPE"),
    make_shared<LinkBuiltin>(LinkBuiltin("SELF_TYPE")),
    {make_shared<Formal>(Formal(StringAttr("x"), StringAttr("String")))}
};

FuncFeature builtin::OutInt = {
    StringAttr("out_int"),
    StringAttr("SELF_TYPE"),
    make_shared<LinkBuiltin>(LinkBuiltin("SELF_TYPE")),
    {make_shared<Formal>(Formal(StringAttr("x"), StringAttr("Int")))}
};

FuncFeature builtin::InString = {
    StringAttr("in_string"),
    StringAttr("String"),
    make_shared<LinkBuiltin>(LinkBuiltin("String")),
    {}
};

FuncFeature builtin::InInt = {
    StringAttr("in_int"),
    StringAttr("Int"),
    make_shared<LinkBuiltin>(LinkBuiltin("Int")),
    {}
};

Class builtin::IO = {
    StringAttr("IO"),
    StringAttr("Object"),
    {
        make_shared<FuncFeature>(OutString),
        make_shared<FuncFeature>(OutInt),
        make_shared<FuncFeature>(InString),
        make_shared<FuncFeature>(InInt),
    },
    {},
};

Class builtin::Int = {
    StringAttr("Int"),
    StringAttr("Object"),
    {},
    {}
};

FuncFeature builtin::Length = {
    StringAttr("length"),
    StringAttr("Int"),
    make_shared<LinkBuiltin>(LinkBuiltin("Int")),
    {}
};

FuncFeature builtin::Concat = {
    StringAttr("String"),
    StringAttr("String"),
    make_shared<LinkBuiltin>(LinkBuiltin("String")),
    {}
};

FuncFeature builtin::Substr = {
    StringAttr("substr"),
    StringAttr("String"),
    make_shared<LinkBuiltin>(LinkBuiltin("String")),
    {
        make_shared<Formal>(Formal(StringAttr("i"), StringAttr("Int"))),
        make_shared<Formal>(Formal(StringAttr("l"), StringAttr("Int"))),
    }
};

Class builtin::String = {
    StringAttr("String"),
    StringAttr("Object"),
    {
        make_shared<FuncFeature>(Length),
        make_shared<FuncFeature>(Concat),
        make_shared<FuncFeature>(Substr),
    },
    {},
};

Class builtin::Bool = {
    StringAttr("Bool"),
    StringAttr("Object"),
    {},
    {}
};

unordered_set<string> builtin::BuiltinClassSet = {
    "Object",
    "IO",
    "Int",
    "String",
    "Bool"
};

unordered_set<string> builtin::InheritableBuiltInClasses = {
    "Object",
    "IO"
};

vector<Class> builtin::BuiltinClasses = {
    Object,
    IO,
    Int,
    String,
    Bool
};