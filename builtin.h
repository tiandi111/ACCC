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

extern repr::FuncFeature Abort;
extern repr::FuncFeature TypeName;
extern repr::FuncFeature Copy;
extern repr::Class Object;

extern repr::FuncFeature OutString;
extern repr::FuncFeature OutInt;
extern repr::FuncFeature InString;
extern repr::FuncFeature InInt;
extern repr::Class IO;

extern repr::Class Int;

extern repr::FuncFeature Length;
extern repr::FuncFeature Concat;
extern repr::FuncFeature Substr;
extern repr::Class String;

extern repr::Class Bool;

extern unordered_set<string> BuiltinClassSet;

} // builtin

} // cool

#endif //COOL_BUILTIN_H
