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

repr::FuncFeature builtin::GetAbortFuncFeature() {
    return {
        StringAttr("abort"),
        StringAttr("Object"),
        make_shared<LinkBuiltin>(LinkBuiltin()),
            {}
    };
}

repr::FuncFeature builtin::GetTypeNameFuncFeature() {
    return {
        StringAttr("type_name"),
        StringAttr("String"),
        make_shared<LinkBuiltin>(LinkBuiltin()),
        {}
    };
}

repr::FuncFeature builtin::GetCopyFuncFeature() {
    return {
        StringAttr("copy"),
        StringAttr("SELF_TYPE"),
        make_shared<LinkBuiltin>(LinkBuiltin()),
        {}
    };
}

repr::Class builtin::GetObjectClass() {
    return {
        StringAttr("Object"),
            StringAttr(""),
            {
//        make_shared<FuncFeature>(Abort),
//        make_shared<FuncFeature>(TypeName),
//        make_shared<FuncFeature>(Copy),
            },
            {},
    };
}

repr::FuncFeature builtin::GetOutStringFuncFeature() {
    return {
        StringAttr("out_string"),
        StringAttr("SELF_TYPE"),
        make_shared<LinkBuiltin>(LinkBuiltin{
            "out_string",
            "SELF_TYPE",
            {"x"}
        }),
        {make_shared<Formal>(Formal(StringAttr("x"), StringAttr("String")))}
    };
}

repr::FuncFeature builtin::GetOutIntFuncFeature() {
    return {
        StringAttr("out_int"),
        StringAttr("SELF_TYPE"),
        make_shared<LinkBuiltin>(LinkBuiltin{
            "out_int",
            "SELF_TYPE",
            {"x"}
        }),
        {make_shared<Formal>(Formal(StringAttr("x"), StringAttr("Int")))}
    };
}

repr::FuncFeature builtin::GetInStringFuncFeature() {
    return {
        StringAttr("in_string"),
        StringAttr("String"),
        make_shared<LinkBuiltin>(LinkBuiltin()),
        {}
    };
}

repr::FuncFeature builtin::GetInIntFuncFeature() {
    return {
        StringAttr("in_int"),
        StringAttr("Int"),
        make_shared<LinkBuiltin>(LinkBuiltin()),
        {}
    };
}

repr::Class builtin::GetIOClass() {
    return {
        StringAttr("IO"),
        StringAttr("Object"),
        {
            make_shared<FuncFeature>(GetOutStringFuncFeature()),
            make_shared<FuncFeature>(GetOutIntFuncFeature()),
//        make_shared<FuncFeature>(InString),
//        make_shared<FuncFeature>(InInt),
        },
        {},
    };
}

repr::Class builtin::GetIntClass() {
    return {
        StringAttr("Int"),
        StringAttr("Object"),
        {},
        {}
    };
}

repr::FuncFeature builtin::GetLengthFuncFeature() {
    return {
        StringAttr("length"),
        StringAttr("Int"),
        make_shared<LinkBuiltin>(LinkBuiltin()),
        {}
    };
}

repr::FuncFeature builtin::GetConcatFuncFeature() {
    return {
        StringAttr("String"),
        StringAttr("String"),
        make_shared<LinkBuiltin>(LinkBuiltin()),
        {}
    };
}

repr::FuncFeature builtin::GetSubstrFuncFeature() {
    return {
        StringAttr("substr"),
        StringAttr("String"),
        make_shared<LinkBuiltin>(LinkBuiltin()),
        {
            make_shared<Formal>(Formal(StringAttr("i"), StringAttr("Int"))),
            make_shared<Formal>(Formal(StringAttr("l"), StringAttr("Int"))),
        }
    };
}

repr::Class builtin::GetStringClass() {
    return {
        StringAttr("String"),
        StringAttr("Object"),
        {
//        make_shared<FuncFeature>(Length),
//        make_shared<FuncFeature>(Concat),
//        make_shared<FuncFeature>(Substr),
        },
        {},
    };
}

repr::Class builtin::GetBoolClass() {
    return {
        StringAttr("Bool"),
        StringAttr("Object"),
        {},
        {}
    };
}

unordered_set<string> builtinClassNames = {
    "Object",
    "IO",
    "Int",
    "String",
    "Bool"
};

unordered_set<string> builtin::GetBuiltinClassNames() {
    return unordered_set<string>(builtinClassNames.begin(), builtinClassNames.end());
}

unordered_set<string> inheritableClasses = {
    "Object",
    "IO",
};

unordered_set<string> builtin::GetInheritableBuiltInClasseNames() {
    return unordered_set<string>(inheritableClasses.begin(), inheritableClasses.end());
}

vector<repr::Class> builtin::GetBuiltinClasses() {
    return {
        GetObjectClass(),
        GetIOClass(),
        GetIntClass(),
        GetStringClass(),
        GetBoolClass()
    };
}

bool builtin::IsBuiltinClass(const string& name) {
    return builtinClassNames.find(name) != builtinClassNames.end();
}

bool builtin::IsInheritable(const string& name) {
    return inheritableClasses.find(name) != inheritableClasses.end();
}