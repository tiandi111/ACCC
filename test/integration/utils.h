//
// Created by 田地 on 2021/8/14.
//

#ifndef COOL_UTILS_H
#define COOL_UTILS_H

#include <fstream>
#include <string>

#include "../../include/json.hpp"

namespace cool {

namespace integration {

using namespace std;

using json = nlohmann::json;

int TotalNumOfTestCases = 0;
int TotalNumOfPassedCases = 0;

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
} // namespace integration

} // namespace cool



#endif //COOL_UTILS_H
