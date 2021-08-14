//
// Created by 田地 on 2021/7/12.
//

#include <istream>
#include <sstream>

#include "semantics.h"
#include "../../frontend/analysis.h"
#include "../../frontend/tokenizer.h"
#include "../../frontend/parser.h"

using namespace std;
using namespace cool;
using namespace integration;
using namespace diag;
using namespace tok;
using namespace parser;
using namespace pass;
using namespace ana;

void cool::integration::RunSemantics(const Test& test) {
    for (auto& case_ : test.cases) {
        Diagnosis diagnosis;
        stringstream sstream(case_.program);
        Tokenizer tokenizer(diagnosis);
        Parser parser(diagnosis, tokenizer.Tokenize(test.title, sstream));
        auto prog = parser.ParseProgram();
        PassContext passContext(diagnosis);
        PassManager::Refresh();
        PassManager::Register<SemanticChecking>();
        PassManager::Run(prog, passContext);
        if (!diagnosis.Empty()) {
            cout<< "-------------- " << test.title << ": " << case_.title << " OUTPUT --------------" <<endl;
            diagnosis.Output(cout);
            cout<< "-------------- " << test.title << ": " << case_.title << " END --------------" <<endl;
        }
        TotalNumOfTestCases ++;
        if (case_.pass != diagnosis.Empty()) cerr << test.title << ": " << case_.title << " (FAIL)" << endl;
        else {
            TotalNumOfPassedCases ++;
            cout << test.title << ": " << case_.title << " (PASS)" << endl;
        }
        cout<<endl;
    }
}

void cool::integration::TestArithmetic() {
    RunSemantics(IntegrationTestLoader::GetIntegrationTestLoader().Get("TestArithmetic"));
}

void cool::integration::TestAssignment() {
    RunSemantics(IntegrationTestLoader::GetIntegrationTestLoader().Get("TestAssignment"));
}

void cool::integration::TestBlock() {
    RunSemantics(IntegrationTestLoader::GetIntegrationTestLoader().Get("TestBlock"));
}

void cool::integration::TestCase() {
    RunSemantics(IntegrationTestLoader::GetIntegrationTestLoader().Get("TestCase"));
}

void cool::integration::TestClassRedefinition() {
    RunSemantics(IntegrationTestLoader::GetIntegrationTestLoader().Get("TestClassRedefinition"));
}

void cool::integration::TestDispatch() {
    RunSemantics(IntegrationTestLoader::GetIntegrationTestLoader().Get("TestDispatch"));
}

void cool::integration::TestIf() {
    RunSemantics(IntegrationTestLoader::GetIntegrationTestLoader().Get("TestIf"));
}

void cool::integration::TestInheritance() {
    RunSemantics(IntegrationTestLoader::GetIntegrationTestLoader().Get("TestInheritance"));
}

void cool::integration::TestInheritBuiltinClass() {
    RunSemantics(IntegrationTestLoader::GetIntegrationTestLoader().Get("TestInheritBuiltinClass"));
}

void cool::integration::TestIsVoid() {
    RunSemantics(IntegrationTestLoader::GetIntegrationTestLoader().Get("TestIsVoid"));
}

void cool::integration::TestLet() {
    RunSemantics(IntegrationTestLoader::GetIntegrationTestLoader().Get("TestLet"));
}

void cool::integration::TestNew() {
    RunSemantics(IntegrationTestLoader::GetIntegrationTestLoader().Get("TestNew"));
}

void cool::integration::TestSelfTypeUsage() {
    RunSemantics(IntegrationTestLoader::GetIntegrationTestLoader().Get("TestSelfTypeUsage"));
}

void cool::integration::TestWhile() {
    RunSemantics(IntegrationTestLoader::GetIntegrationTestLoader().Get("TestWhile"));
}

int main() {
    TestArithmetic();
    TestAssignment();
    TestBlock();
    TestCase();
    TestClassRedefinition();
    TestDispatch();
    TestIf();
    TestInheritance();
    TestInheritBuiltinClass();
    TestIsVoid();
    TestLet();
    TestNew();
    TestSelfTypeUsage();
    TestWhile();
    cout<< "Test Complete: " << "(" << TotalNumOfPassedCases << "/" << TotalNumOfTestCases << ") Passed" <<endl;
}

