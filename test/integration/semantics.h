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

#include "../../include/json.hpp"
#include "../../frontend/diag.h"

using namespace std;

using json = nlohmann::json;

namespace cool {

namespace integration {

using namespace diag;

struct Case {
    string title;
    string program;
    bool pass;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Case, title, program, pass);

struct Test {
    string title;
    vector<Case> cases;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Test, title, cases);

class IntegrationTestLoader {
  private:
    json j;

    void load(istream& in) { in >> j; }

  public:
    IntegrationTestLoader(const string& filename) {
        fstream file;
        file.open(filename, ios::in);
        assert(file);
        load(file);
    }

    static IntegrationTestLoader& GetIntegrationTestLoader() {
        static IntegrationTestLoader integrationTestLoader("../test/data/semantics.json");
        return integrationTestLoader;
    }

    Test Get(const string& title) {
        return j.at(title).get<Test>();
    }
};

int TotalNumOfTestCases = 0;
int TotalNumOfPassedCases = 0;

void RunFrontEnd(const Test& test);

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
