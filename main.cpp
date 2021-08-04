//
// Created by 田地 on 2021/5/30.
//

#include <iostream>
#include <sstream>
#include <fstream>

#include "frontend/diag.h"
#include "frontend/tokenizer.h"
#include "frontend/parser.h"
#include "frontend/pass.h"
#include "frontend/analysis.h"
#include "frontend/llvm_gen.h"
#include "frontend/adt.h"

using namespace std;
using namespace cool;
using namespace diag;
using namespace tok;
using namespace parser;
using namespace pass;
using namespace ana;
using namespace irgen;
using namespace adt;

int main() {
    fstream file;
    file.open("../main_data", ios::in);
    assert(file);

    Diagnosis diagnosis;
    Tokenizer tokenizer(diagnosis);
    Parser parser(diagnosis, tokenizer.Tokenize("main_data", file));
    auto prog = parser.ParseProgram();
    if (!diagnosis.Empty()) {
        diagnosis.Output(cerr);
        return 0;
    }
    PassContext passContext(diagnosis);
    PassManager::Refresh();
    PassManager::Register<SemanticChecking>();
    PassManager::Run(*prog, passContext);
    if (!diagnosis.Empty()) {
        diagnosis.Output(cerr);
        return 0;
    }
    LLVMGen llvmGen(*passContext.Get<ScopedTableSpecializer<SymbolTable>>("symbol_table"));
    llvmGen.Visit(*prog);
    llvmGen.DumpTextualIR("output.ll");
//    llvmGen.EmitObjectFile("output.o");
    diagnosis.Output(cerr);
}

