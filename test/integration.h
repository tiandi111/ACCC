//
// Created by 田地 on 2021/7/12.
//

#ifndef COOL_INTEGRATION_H
#define COOL_INTEGRATION_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>

#include "../include/json.hpp"
#include "../diag.h"

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
    vector<Case> cases;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Test, cases);

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
        static IntegrationTestLoader integrationTestLoader("../test/data/integration.json");
        return integrationTestLoader;
    }

    Test Get(const string& title) {
        return j.at(title).get<Test>();
    }
};



void RunFrontEnd(const string& title, Diagnosis& diagnosis, istream& in);

void TestInheritBuiltinClass();
void TestSelfTypeUsage();

} // namespace integration

} // namespace cool

#endif //COOL_INTEGRATION_H
