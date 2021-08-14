//
// Created by 田地 on 2021/7/12.
//

#ifndef COOL_SEMANTICS_H
#define COOL_SEMANTICS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>

#include "utils.h"
#include "../../frontend/diag.h"

using namespace std;

namespace cool {

namespace integration {

using namespace diag;

void RunSemantics(const Test& test);

void TestArithmetic();
void TestAssignment();
void TestBlock();
void TestCase();
void TestClassRedefinition();
void TestDispatch();
void TestIf();
void TestInheritance();
void TestInheritBuiltinClass();
void TestIsVoid();
void TestLet();
void TestNew();
void TestSelfTypeUsage();
void TestWhile();

} // namespace integration

} // namespace cool

#endif //COOL_SEMANTICS_H
