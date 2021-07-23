//
// Created by 田地 on 2021/5/30.
//

#include <iostream>
#include <sstream>

#include "frontend/diag.h"
#include "frontend/tokenizer.h"
#include "frontend/parser.h"
#include "frontend/pass.h"
#include "frontend/analysis.h"
#include "frontend/llvm_gen.h"

using namespace std;
using namespace cool;
using namespace diag;
using namespace tok;
using namespace parser;
using namespace pass;
using namespace ana;
using namespace irgen;

int main() {
    Diagnosis diagnosis;
    stringstream sstream("class A {};");
    Tokenizer tokenizer(diagnosis);
    Parser parser(diagnosis, tokenizer.Tokenize("test", sstream));
    auto prog = parser.ParseProgram();
    PassContext passContext(diagnosis);
    PassManager::Refresh();
    PassManager::Register<SemanticChecking>();
    PassManager::Run(prog, passContext);
    LLVMGen llvmGen;
    llvmGen.Visit(prog);
    llvmGen.DumpTextualIR("output.ll");
    llvmGen.EmitObjectFile("output.o");
    diagnosis.Output(cerr);
}

