//
// Created by 田地 on 2021/7/12.
//

#include <istream>
#include <sstream>

#include "integration.h"
#include "../analysis.h"
#include "../diag.h"
#include "../tokenizer.h"
#include "../parser.h"
#include "../pass.h"

using namespace std;
using namespace cool;
using namespace integration;
using namespace diag;
using namespace tok;
using namespace parser;
using namespace pass;
using namespace ana;

void cool::integration::RunFrontEnd(const string& title, Diagnosis& diagnosis, istream& in) {
    Tokenizer tokenizer(diagnosis);
    Parser parser(diagnosis, tokenizer.Tokenize(title, in));
    auto prog = parser.ParseProgram();
    PassContext passContext(diagnosis);
    PassManager::Refresh();
    PassManager::Register<InstallBuiltin>();
    PassManager::Register<InitSymbolTable>();
    PassManager::Register<BuildInheritanceTree>();
    PassManager::Register<TypeChecking>();
    PassManager::Run(prog, passContext);
}

void cool::integration::TestInheritBuiltinClass() {
    auto loader = IntegrationTestLoader::GetIntegrationTestLoader();
    auto test = loader.Get("TestInheritBuiltinClass");
    for (auto& case_ : test.cases) {
        Diagnosis diagnosis;
        stringstream sstream;
        sstream << case_.program;
        RunFrontEnd("TestInheritBuiltinClass", diagnosis, sstream);
        if (case_.pass != diagnosis.Empty()) cerr<< "TestInheritBuiltinClass: " << case_.title << " failed" <<endl;
    }
}

void cool::integration::TestSelfTypeUsage() {
    auto loader = IntegrationTestLoader::GetIntegrationTestLoader();
    auto test = loader.Get("TestSelfTypeUsage");
    for (auto& case_ : test.cases) {
        Diagnosis diagnosis;
        stringstream sstream;
        sstream << case_.program;
        RunFrontEnd("TestSelfTypeUsage", diagnosis, sstream);
        if (case_.pass != diagnosis.Empty()) {
            diagnosis.Output(cerr);
            cerr << "TestSelfTypeUsage: " << case_.title << " failed" << endl;
        }
    }
}

int main() {
    TestInheritBuiltinClass();
    TestSelfTypeUsage();
}

