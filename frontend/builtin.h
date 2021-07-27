//
// Created by 田地 on 2021/6/23.
//

#ifndef COOL_BUILTIN_H
#define COOL_BUILTIN_H

#include <vector>
#include <unordered_set>
#include <string>

#include "repr.h"

using namespace std;

namespace cool {

namespace builtin {

repr::FuncFeature GetAbortFuncFeature();
repr::FuncFeature GetTypeNameFuncFeature();
repr::FuncFeature GetCopyFuncFeature();
repr::Class GetObjectClass();

repr::FuncFeature GetOutStringFuncFeature();
repr::FuncFeature GetOutIntFuncFeature();
repr::FuncFeature GetInStringFuncFeature();
repr::FuncFeature GetInIntFuncFeature();
repr::Class GetIOClass();

repr::Class GetIntClass();

repr::FuncFeature GetLengthFuncFeature();
repr::FuncFeature GetConcatFuncFeature();
repr::FuncFeature GetSubstrFuncFeature();
repr::Class GetStringClass();

repr::Class GetBoolClass();

unordered_set<string> GetBuiltinClassNames();
unordered_set<string> GetInheritableBuiltInClasseNames();
vector<repr::Class> GetBuiltinClasses();

bool IsBuiltinClass(const string& name);
bool IsInheritable(const string& name);

} // builtin

} // cool

#endif //COOL_BUILTIN_H
