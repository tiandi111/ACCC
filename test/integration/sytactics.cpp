//
// Created by 田地 on 2021/8/14.
//

#include <sstream>

#include "syntactics.h"
#include "../../frontend/diag.h"
#include "../../frontend/parser.h"
#include "../../frontend/tokenizer.h"

using namespace cool;
using namespace diag;
using namespace tok;
using namespace parser;

void cool::integration::RunSyntactics(const Test& test) {
    for (auto& case_ : test.cases) {
        Diagnosis diagnosis;
        stringstream sstream(case_.program);
        Tokenizer tokenizer(diagnosis);
        Parser parser(diagnosis, tokenizer.Tokenize(test.title, sstream));
        parser.ParseProgram();
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
