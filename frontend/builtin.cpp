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

repr::FuncFeature* builtin::GetAbortFuncFeature() {
    return new FuncFeature(
        StringAttr("abort"),
        StringAttr("Object"),
        new LinkBuiltin()
    );
}

repr::FuncFeature* builtin::GetTypeNameFuncFeature() {
    return new FuncFeature(
        StringAttr("type_name"),
        StringAttr("String"),
        new LinkBuiltin(),
        {}
    );
}

repr::FuncFeature* builtin::GetCopyFuncFeature() {
    return new FuncFeature(
        StringAttr("copy"),
        StringAttr("SELF_TYPE"),
        new LinkBuiltin(),
        {}
    );
}

repr::Class* builtin::GetObjectClass() {
    return new Class(
        StringAttr("Object"),
        StringAttr(""),
        {
//        make_shared<FuncFeature>(Abort),
//        make_shared<FuncFeature>(TypeName),
//        make_shared<FuncFeature>(Copy),
        },
        {}
    );
}

repr::FuncFeature* builtin::GetOutStringFuncFeature() {
    return new FuncFeature(
        StringAttr("out_string"),
        StringAttr("SELF_TYPE"),
        new LinkBuiltin(
            "out_string",
            "SELF_TYPE",
            {"x"}
        ),
        {
            new Formal(StringAttr("x"), StringAttr("String"))
        }
    );
}

repr::FuncFeature* builtin::GetOutIntFuncFeature() {
    return new FuncFeature(
        StringAttr("out_int"),
        StringAttr("SELF_TYPE"),
        new LinkBuiltin(
            "out_int",
            "SELF_TYPE",
            {"x"}
        ),
        {
            new Formal(StringAttr("x"), StringAttr("Int"))
        }
    );
}

repr::FuncFeature* builtin::GetInStringFuncFeature() {
    return new FuncFeature(
        StringAttr("in_string"),
        StringAttr("String"),
        new LinkBuiltin(),
        {}
    );
}

repr::FuncFeature* builtin::GetInIntFuncFeature() {
    return new FuncFeature(
        StringAttr("in_int"),
        StringAttr("Int"),
        new LinkBuiltin(),
        {}
    );
}

repr::Class* builtin::GetIOClass() {
    return new Class(
        StringAttr("IO"),
        StringAttr("Object"),
        {
            GetOutStringFuncFeature(),
            GetOutIntFuncFeature(),
//        make_shared<FuncFeature>(InString),
//        make_shared<FuncFeature>(InInt),
        },
        {}
    );
}

repr::Class* builtin::GetIntClass() {
    return new Class(
        StringAttr("Int"),
        StringAttr("Object"),
        {},
        {}
    );
}

repr::FuncFeature* builtin::GetLengthFuncFeature() {
    return new FuncFeature(
        StringAttr("length"),
        StringAttr("Int"),
        new LinkBuiltin(),
        {}
    );
}

repr::FuncFeature* builtin::GetConcatFuncFeature() {
    return new FuncFeature(
        StringAttr("String"),
        StringAttr("String"),
        new LinkBuiltin(),
        {}
    );
}

repr::FuncFeature* builtin::GetSubstrFuncFeature() {
    return new FuncFeature(
        StringAttr("substr"),
        StringAttr("String"),
        new LinkBuiltin(),
        {
            new Formal(StringAttr("i"), StringAttr("Int")),
            new Formal(StringAttr("l"), StringAttr("Int")),
        }
    );
}

repr::Class* builtin::GetStringClass() {
    return new Class(
        StringAttr("String"),
        StringAttr("Object"),
        {
//        make_shared<FuncFeature>(Length),
//        make_shared<FuncFeature>(Concat),
//        make_shared<FuncFeature>(Substr),
        },
        {}
    );
}

repr::Class* builtin::GetBoolClass() {
    return new Class(
        StringAttr("Bool"),
        StringAttr("Object"),
        {},
        {}
    );
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

vector<repr::Class*> builtin::NewBuiltinClasses() {
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

string builtin::CoolMainFunctionName() {
    return "coolmain";
}