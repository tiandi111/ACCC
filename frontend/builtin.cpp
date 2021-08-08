//
// Created by 田地 on 2021/6/23.
//

#include <memory>

#include "builtin.h"
#include "repr.h"
#include "constant.h"

using namespace std;
using namespace cool;
using namespace builtin;
using namespace repr;
using namespace constant;

repr::FuncFeature* builtin::GetAbortFuncFeature() {
    return new FuncFeature(
        StringAttr("abort"),
        StringAttr(CLS_OBJECT_NAME),
        new LinkBuiltin()
    );
}

repr::FuncFeature* builtin::GetTypeNameFuncFeature() {
    return new FuncFeature(
        StringAttr("type_name"),
        StringAttr(CLS_STRING_NAME),
        new LinkBuiltin(),
        {}
    );
}

repr::FuncFeature* builtin::GetCopyFuncFeature() {
    return new FuncFeature(
        StringAttr("copy"),
        StringAttr(TYPE_SELF_TYPE),
        new LinkBuiltin(),
        {}
    );
}

repr::Class* builtin::GetObjectClass() {
    return new Class(
        StringAttr(CLS_OBJECT_NAME),
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
        StringAttr(TYPE_SELF_TYPE),
        new LinkBuiltin(
            "out_string",
            TYPE_SELF_TYPE,
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
        StringAttr(TYPE_SELF_TYPE),
        new LinkBuiltin(
            "out_int",
            TYPE_SELF_TYPE,
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
        StringAttr(CLS_STRING_NAME),
        new LinkBuiltin(),
        {}
    );
}

repr::FuncFeature* builtin::GetInIntFuncFeature() {
    return new FuncFeature(
        StringAttr("in_int"),
        StringAttr(CLS_INT_NAME),
        new LinkBuiltin(),
        {}
    );
}

repr::Class* builtin::GetIOClass() {
    return new Class(
        StringAttr(CLS_IO_NAME),
        StringAttr(CLS_OBJECT_NAME),
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
        StringAttr(CLS_INT_NAME),
        StringAttr(CLS_OBJECT_NAME),
        {},
        {}
    );
}

repr::FuncFeature* builtin::GetLengthFuncFeature() {
    return new FuncFeature(
        StringAttr("length"),
        StringAttr(CLS_INT_NAME),
        new LinkBuiltin(),
        {}
    );
}

repr::FuncFeature* builtin::GetConcatFuncFeature() {
    return new FuncFeature(
        StringAttr(CLS_STRING_NAME),
        StringAttr(CLS_STRING_NAME),
        new LinkBuiltin(),
        {}
    );
}

repr::FuncFeature* builtin::GetSubstrFuncFeature() {
    return new FuncFeature(
        StringAttr("substr"),
        StringAttr(CLS_STRING_NAME),
        new LinkBuiltin(),
        {
            new Formal(StringAttr("i"), StringAttr("Int")),
            new Formal(StringAttr("l"), StringAttr("Int")),
        }
    );
}

repr::Class* builtin::GetStringClass() {
    return new Class(
        StringAttr(CLS_STRING_NAME),
        StringAttr(CLS_OBJECT_NAME),
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
        StringAttr(CLS_BOOL_NAME),
        StringAttr(CLS_OBJECT_NAME),
        {},
        {}
    );
}

unordered_set<string> builtinClassNames = {
    CLS_OBJECT_NAME,
    CLS_IO_NAME,
    CLS_INT_NAME,
    CLS_STRING_NAME,
    CLS_BOOL_NAME
};

unordered_set<string> builtin::GetBuiltinClassNames() {
    return unordered_set<string>(builtinClassNames.begin(), builtinClassNames.end());
}

unordered_set<string> inheritableClasses = {
    CLS_OBJECT_NAME,
    CLS_IO_NAME,
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