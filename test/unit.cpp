//
// Created by 田地 on 2021/6/2.
//
#include <iostream>
#include <sstream>
#include <fstream>
#include "unit.h"
#include "../parser.h"
#include "../tokenizer.h"
#include "../pass.h"
#include "../analysis.h"
#include "../stable.h"
#include "../builtin.h"
#include "../typead.h"
#include "../diag.h"

using namespace std;

using namespace cool;
using namespace parser;
using namespace repr;
using namespace pass;
using namespace ana;
using namespace builtin;
using namespace attr;
using namespace diag;
using namespace tok;

Diagnosis testDiag;

void TestMatchMultiple() {
    Parser parser(testDiag, {
        {Token::ID, "test", "test", 0},
        {Token::TypeID, "test", "test", 0}
    });
    vector<Token::Type> types = {Token::ID, Token::TypeID};
    assert(parser.MatchMultiple(types));
}

void TestParseID() {
    Parser parser(testDiag, {
        {Token::ID, "test", "test", 0},
        {Token::ID, "test", "test", 0}
    });
    try {
        auto id = parser.ParseID();
        auto id1 = parser.ParseID();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseUnary() {
    Parser parser(testDiag, {
        {Token::kNot, "", "", 0},
        {Token::ID, "test", "test", 0}
    });
    try {
        auto id = parser.ParseNot();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseBinary() {
    Parser parser(testDiag, {
        {Token::kAdd, "", "", 0},
        {Token::ID, "test", "test", 0}
    });
    try {
        ID id;
        auto binary = parser.ParseAdd(make_shared<ID>(id));
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseNew() {
    Parser parser(testDiag, {
        {Token::kNew, "", "", 0},
        {Token::TypeID, "test", "test", 0}
    });
    try {
        auto newExpr = parser.ParseNew();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseCall() {
    Parser parser(testDiag, {
        // case 1
        {Token::ID, "", "", 0},
        {Token::kOpenParen, "test", "test", 0},
        {Token::kCloseParen, "test", "test", 0},
        // case 2
        {Token::ID, "", "", 0},
        {Token::kOpenParen, "test", "test", 0},
        {Token::ID, "", "", 0},
        {Token::kCloseParen, "test", "test", 0},
        // case 3
        {Token::ID, "", "", 0},
        {Token::kOpenParen, "test", "test", 0},
        {Token::ID, "", "", 0},
        {Token::kComma, "", "", 0},
        {Token::ID, "", "", 0},
        {Token::kCloseParen, "test", "test", 0}
    });
    try {
        auto call1 = parser.ParseCall();
        auto call2 = parser.ParseCall();
        auto call3 = parser.ParseCall();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseAssign() {
    Parser parser(testDiag, {
        {Token::ID, "test", "test", 0},
        {Token::kAssignment, "", "", 0},
        {Token::ID, "test", "test", 0}
    });
    try {
        auto assign = parser.ParseAssign();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseCase() {
    Parser parser(testDiag, {
        // case 1
        {Token::kCase, "test", "test", 0},
        {Token::ID, "", "", 0},
        {Token::kOf, "test", "test", 0},
        {Token::ID, "test", "test", 0},
        {Token::kColon, "test", "test", 0},
        {Token::TypeID, "test", "test", 0},
        {Token::kEval, "test", "test", 0},
        {Token::ID, "test", "test", 0},
        {Token::kSemiColon, "test", "test", 0},
        {Token::kEsac, "test", "test", 0},
        // case 2
        {Token::kCase, "test", "test", 0},
        {Token::ID, "", "", 0},
        {Token::kOf, "test", "test", 0},
        {Token::ID, "test", "test", 0},
        {Token::kColon, "test", "test", 0},
        {Token::TypeID, "test", "test", 0},
        {Token::kEval, "test", "test", 0},
        {Token::ID, "test", "test", 0},
        {Token::kSemiColon, "test", "test", 0},
        {Token::ID, "test", "test", 0},
        {Token::kColon, "test", "test", 0},
        {Token::TypeID, "test", "test", 0},
        {Token::kEval, "test", "test", 0},
        {Token::ID, "test", "test", 0},
        {Token::kSemiColon, "test", "test", 0},
        {Token::kEsac, "test", "test", 0}
    });
    try {
        auto case1 = parser.ParseCase();
        auto case2 = parser.ParseCase();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseLet() {
    Parser parser(testDiag, {
        // case 1
        {Token::kLet, "", "", 0},
        {Token::ID, "test", "test", 0},
        {Token::kColon, "test", "test", 0},
        {Token::TypeID, "", "", 0},
        {Token::kIn, "test", "test", 0},
        {Token::ID, "", "", 0},
        // case 2
        {Token::kLet, "", "", 0},
        {Token::ID, "test", "test", 0},
        {Token::kColon, "test", "test", 0},
        {Token::TypeID, "", "", 0},
        {Token::kAssignment, "", "", 0},
        {Token::ID, "", "", 0},
        {Token::kIn, "test", "test", 0},
        {Token::ID, "", "", 0},
        // case 3
        {Token::kLet, "", "", 0},
        {Token::ID, "test", "test", 0},
        {Token::kColon, "test", "test", 0},
        {Token::TypeID, "", "", 0},
        {Token::kAssignment, "", "", 0},
        {Token::ID, "", "", 0},
        {Token::kComma, "", "", 0},
        {Token::ID, "test", "test", 0},
        {Token::kColon, "test", "test", 0},
        {Token::TypeID, "", "", 0},
        {Token::kAssignment, "", "", 0},
        {Token::ID, "", "", 0},
        {Token::kIn, "test", "test", 0},
        {Token::ID, "", "", 0},
    });
    try {
        auto let1 = parser.ParseLet();
        auto let2 = parser.ParseLet();
        auto let3 = parser.ParseLet();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseWhile() {
    Parser parser(testDiag, {
        {Token::kWhile, "test", "test", 0},
        {Token::ID, "test", "test", 0},
        {Token::kLoop, "", "", 0},
        {Token::ID, "test", "test", 0},
        {Token::kPool, "test", "test", 0}
    });
    try {
        auto whileExpr = parser.ParseWhile();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseBlock() {
    vector<ParserTestCase> cases = {
        {
            "case1",
            {
                {Token::kOpenBrace, "test", "test", 0},
                {Token::ID, "test", "test", 0},
                {Token::kSemiColon, "", "", 0},
                {Token::kCloseBrace, "test", "test", 0}
            },
            false,
        },
        {
            "case2",
            {
                {Token::kOpenBrace, "test", "test", 0},
                {Token::ID, "test", "test", 0},
                {Token::kSemiColon, "", "", 0},
                {Token::ID, "test", "test", 0},
                {Token::kSemiColon, "", "", 0},
                {Token::kCloseBrace, "test", "test", 0},
            },
            false,
        },
        {
            "case3",
            {
                {Token::kOpenBrace, "test", "test", 0},
                {Token::kCloseBrace, "test", "test", 0},
            },
            true,
        },
    };
    for (auto& tc : cases) {
        cout<< tc.name <<endl;
        try {
            Parser parser(testDiag, tc.toks);
            auto block = parser.ParseBlock();
            if (tc.throwExp) assert(false);
        } catch (exception& e) {
            if (!tc.throwExp) assert(false);
        }
    }
}

void TestParseIf() {
    Parser parser(testDiag, {
        {Token::kIf, "test", "test", 0},
        {Token::ID, "test", "test", 0},
        {Token::kThen, "", "", 0},
        {Token::ID, "test", "test", 0},
        {Token::kElse, "test", "test", 0},
        {Token::ID, "test", "test", 0},
        {Token::kFi, "test", "test", 0},
    });
    try {
        auto ifExpr = parser.ParseIf();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseExpr() {
    Parser parser(testDiag, {
        {Token::ID, "test", "test", 0},
        {Token::kAdd, "", "", 0},
        {Token::ID, "test", "test", 0}
    });
    try {
        auto expr = parser.ParseExpr();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseFormal() {
    Parser parser(testDiag, {
        {Token::ID, "test", "test", 0},
        {Token::kColon, "", "", 0},
        {Token::TypeID, "test", "test", 0}
    });
    try {
        auto formal = parser.ParseFormal();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseFuncFeature() {
    vector<Token> toks;
    for (auto& featureToks : testFuncFeats) {
        for (auto& tok : featureToks) {
            toks.emplace_back(tok);
        }
    }
    Parser parser(testDiag, toks);
    try {
        for (int i = 0; i < testFuncFeats.size(); i++) {
            parser.ParseFuncFeature();
        }
    } catch (exception& e) {
        cout<< e.what() <<endl;
        assert(false);
    }
}

void TestParseFieldFeature() {
    vector<Token> toks;
    for (auto& featureToks : testFieldFeats) {
        for (auto& tok : featureToks) {
            toks.emplace_back(tok);
        }
    }
    Parser parser(testDiag, toks);
    try {
        for (int i = 0; i < testFieldFeats.size(); i++) {
            parser.ParseFieldFeature();
        }
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseClass() {
    vector<Token> toks;
    ConstructClassTokens(toks, true, testFuncFeats, testFieldFeats);
    ConstructClassTokens(toks, false, testFuncFeats, testFieldFeats);
    Parser parser(testDiag, toks);
    try {
        parser.ParseClass();
        parser.ParseClass();
    } catch (exception& e) {
        cerr<< e.what() <<endl;
        assert(false);
    }
}

void TestParseProgram() {
    vector<Token> toks;
    ConstructProgTokens(toks, true, testFuncFeats, testFieldFeats);
    ConstructProgTokens(toks, false , testFuncFeats, testFieldFeats);
    Parser parser(testDiag, toks);
    try {
        parser.ParseProgram();
    } catch (exception& e) {
        assert(false);
    }
}

void TestTokDigit() {
    vector<TokComponentTestCase> cases = {
        {"0", {Token::Integer, "0", "0", 0}},
        {"00", {Token::Integer, "00", "00", 0}},
        {"01", {Token::Integer, "01", "01", 0}},
        {"123", {Token::Integer, "123", "123", 0}},
        {"12a", {Token::Integer, "12", "12", 0}},
    };
    for (auto& c : cases) {
        stringstream sstream;
        sstream<< c.str;
        Tokenizer tokenizer(testDiag);
        auto tok = tokenizer.TokDigit(0, sstream);
        assert(tok.type == c.tok.type && tok.str == c.tok.str && tok.val == c.tok.val);
    }
}

void TestTokAlpha() {
    vector<TokComponentTestCase> cases = {
        {"abc", {Token::ID, "abc", "abc", 0}},
        {"_abc", {Token::ID, "_abc", "_abc", 0}},
        {"ab1_1", {Token::ID, "ab1_1", "ab1_1", 0}},
        {"test space", {Token::ID, "test", "test", 0}},
        {"test\nspace", {Token::ID, "test", "test", 0}},
        {"File", {Token::TypeID, "File", "File", 0}},
        {"File inherits", {Token::TypeID, "File", "File", 0}},
        {"File_Reader1", {Token::TypeID, "File_Reader1", "File_Reader1", 0}},
        {"if", {Token::kIf, "", "", 0}},
        {"If", {Token::kIf, "", "", 0}},
        {"ifabc", {Token::ID, "ifabc", "ifabc", 0}},
    };
    for (auto& c : cases) {
        stringstream sstream;
        sstream<< c.str;
        Tokenizer tokenizer(testDiag);
        auto tok = tokenizer.TokAlpha(0, sstream);
        assert(tok.type == c.tok.type && tok.str == c.tok.str && tok.val == c.tok.val);
    }
}

void TestTokString() {
    vector<TokComponentTestCase> cases = {
        {"\"\"", {Token::String, "", "", 0}},
        {"\"abc\"", {Token::String, "abc", "abc", 0}},
        {"\"abc\"\"", {Token::String, "abc", "abc", 0}},
        {"\"\\\"\"", {Token::String, "\"", "\"", 0}},
        {"\"\\\"\\c\"", {Token::String, "\"c", "\"c", 0}},
    };
    for (auto& c : cases) {
        stringstream sstream;
        sstream<< c.str;
        Tokenizer tokenizer(testDiag);
        auto tok = tokenizer.TokString(0, sstream);
        assert(tok.type == c.tok.type && tok.str == c.tok.str && tok.val == c.tok.val);
    }
}

void TestTokSpecial() {
    vector<TokComponentTestCase> cases = {
        {"<-", {Token::kAssignment, "", "", 0}},
        {"<6", {Token::kLessThan, "", "", 0}},
        {"<=6", {Token::kLessThanOrEqual, "", "", 0}},
        {"=6", {Token::kEqual, "", "", 0}},
        {"=>6", {Token::kEval, "", "", 0}},
        {";", {Token::kSemiColon, "", "", 0}},
    };
    for (auto& c : cases) {
        stringstream sstream;
        sstream<< c.str;
        Tokenizer tokenizer(testDiag);
        auto tok = tokenizer.TokSpecial(0, sstream);
        assert(tok.type == c.tok.type && tok.str == c.tok.str && tok.val == c.tok.val);
    }
}

void TestTokenizer() {
    vector<TokenizerTestCase> cases = {
        {"class B {\n"
         "    s : String <- \"Hello\";\n"
         "    g (y : String) : String {\n"
         "        y.concat(s)\n"
         "    };\n"
         ", 0};", {
            {Token::kClass, "", "", 0},
            {Token::TypeID, "B", "B", 0},
            {Token::kOpenBrace, "", "", 0},
            {Token::ID, "s", "s", 0},
            {Token::kColon, "", "", 0},
            {Token::TypeID, "String", "String", 0},
            {Token::kAssignment, "", "", 0},
            {Token::String, "Hello", "Hello", 0},
            {Token::kSemiColon, "", "", 0},
            {Token::ID, "g", "g", 0},
            {Token::kOpenParen, "", "", 0},
            {Token::ID, "y", "y", 0},
            {Token::kColon, "", "", 0},
            {Token::TypeID, "String", "String", 0},
            {Token::kCloseParen, "", "", 0},
            {Token::kColon, "", "", 0},
            {Token::TypeID, "String", "String", 0},
            {Token::kOpenBrace, "", "", 0},
            {Token::ID, "y", "y", 0},
            {Token::kDot, "", "", 0},
            {Token::ID, "concat", "concat", 0},
            {Token::kOpenParen, "", "", 0},
            {Token::ID, "s", "s", 0},
            {Token::kCloseParen, "", "", 0},
            {Token::kCloseBrace, "", "", 0},
            {Token::kSemiColon, "", "", 0},
            {Token::kCloseBrace, "", "", 0},
            {Token::kSemiColon, "", "", 0}
        }},
    };
    for (auto& c : cases) {
        stringstream sstream(c.str);
        Tokenizer tokenizer(testDiag);
        auto toks = tokenizer.Tokenize(sstream);
        assert(c.toks.size() == toks.size());
        for (int i = 0; i < c.toks.size(); i++) {
            assert(c.toks[i].type == toks[i].type &&
            c.toks[i].str == toks[i].str &&
            c.toks[i].val == toks[i].val);
        }
    }
}

void TestRegisterPass() {
    PassManager::Refresh();
    class TestPass : public ProgramPass {
        void Required() {}
    };
    PassManager::Register<TestPass>();
    assert(dynamic_pointer_cast<TestPass>(PassManager::Get<TestPass>()));
    try {
        PassManager::Register<TestPass>();
        assert(false);
    } catch (exception& e) {}
}

void TestRequiredPass() {
    PassManager::Refresh();
    class TestPassA : public ProgramPass {
        void Required() {}
    };
    class TestPassB : public ProgramPass {
        void Required() {}
    };
    PassManager::Register<TestPassA>();
    PassManager::Register<TestPassB>();
    PassManager::Required<TestPassA, TestPassB>();
}

void TestPassManager() {
    {
        PassManager::Refresh();
        class TestPassA : public ProgramPass {
            void Required() {}
        };
        class TestPassC;
        class TestPassB : public ProgramPass {
            void Required() {
                PassManager::Required<TestPassB, TestPassA>();
                PassManager::Required<TestPassB, TestPassC>();
            }
        };
        class TestPassC : public ProgramPass {
            void Required() {
                PassManager::Required<TestPassC, TestPassA>();
            }
        };
        class TestPassD : public ProgramPass {
            void Required() {
                PassManager::Required<TestPassD, TestPassB>();
                PassManager::Required<TestPassD, TestPassC>();
            }
        };
        PassManager::Register<TestPassA>();
        PassManager::Register<TestPassB>();
        PassManager::Register<TestPassC>();
        PassManager::Register<TestPassD>();
        PassManager::Run();
    }
    {
        PassManager::Refresh();
        class TestPassB;
        class TestPassA : public ProgramPass {
            void Required() {
                PassManager::Required<TestPassA, TestPassB>();
            }
        };
        class TestPassB : public ProgramPass {
            void Required() {
                PassManager::Required<TestPassB, TestPassA>();
            }
        };
        PassManager::Register<TestPassA>();
        PassManager::Register<TestPassB>();
        try {
            PassManager::Run();
            assert(false);
        } catch (exception& e) {}
    }
}

void TestVirtualTable() {
    class Base { public: int state = 0; virtual ~Base() {} };
    class Derived1 : public Base { public: ~Derived1() override {} };
    class Derived2 : public Base { public: ~Derived2() override {} };

    VirtualTable<void, Base> vtable;

    auto f1 = [](Derived1& base){ base.state = 1; };
    auto f2 = [](Derived2& base){ base.state = 2; };

    vtable.SetDispatch<Derived1&>([&](Base& base){
        return f1(static_cast<Derived1&>(base));
    });
    vtable.SetDispatch<Derived2&>([&](Base& base){
        return f2(static_cast<Derived2&>(base));
    });

    Derived1 derived1;
    Derived2 derived2;

    try {
        vtable(derived1);
        assert(derived1.state == 1);
        vtable(derived2);
        assert(derived2.state == 2);
    } catch (exception& e) {
        cerr<< e.what() <<endl;
        assert(false);
    }
}

void TestSemanticCheckingPasses() {
    vector<Token> toks;
    ConstructProgTokens(toks, false, testFuncFeats, testFieldFeats);
    Parser parser(testDiag, toks);
    auto prog = parser.ParseProgram();
    int clsNum = prog.classes.size();
    PassContext passContext(testDiag);
    try {
        // Test InstallBuiltin Pass
        InstallBuiltin installBuiltin;
        installBuiltin(prog, passContext);
        assert(prog.classes.size() == clsNum + BuiltinClassSet.size());

        // Test InitSymbolTable Pass
        InitSymbolTable initSymbolTable;
        initSymbolTable(prog, passContext);
        assert(passContext.Get<ScopedTableSpecializer<SymbolTable>>("symbol_table"));

        // Test BuildInheritanceTree Pass
        BuildInheritanceTree buildInheritanceTree;
        buildInheritanceTree(prog, passContext);
        auto typeAdvisor = *passContext.Get<TypeAdvisor>("type_advisor");
        for (auto& cls : prog.classes) {
            typeAdvisor.Conforms(cls->name, "Object");
        }

        // Test TypeChecking Pass
        TypeChecking typeChecking;
        typeChecking(prog, passContext);

    } catch (exception& e) {
        cerr<< e.what() <<endl;
        assert(false);
    }
}

void TestFrontEnd() {
    fstream file;
    file.open("data/t", ios::in);
    assert(file);
    Tokenizer tokenizer(testDiag);
    Parser parser(testDiag, tokenizer.Tokenize(file));
    auto prog = parser.ParseProgram();
    pass::PassManager::Register<ana::InstallBuiltin>();
    pass::PassManager::Register<ana::InitSymbolTable>();
    pass::PassManager::Register<ana::BuildInheritanceTree>();
    pass::PassManager::Register<ana::TypeChecking>();
    pass::PassManager::GetPassManager().Run();
}


int main() {
//    TestMatchMultiple();
//
//    TestParseID();
//    TestParseUnary();
//    TestParseBinary();
//    TestParseNew();
//    TestParseCall();
//    TestParseAssign();
//    TestParseCase();
//    TestParseLet();
//    TestParseWhile();
//    TestParseBlock();
//    TestParseIf();
//
//    TestParseExpr();
//    TestParseFormal();
//    TestParseFuncFeature();
//    TestParseFieldFeature();
//    TestParseClass();
//    TestParseProgram();
//
//    TestTokDigit();
//    TestTokAlpha();
//    TestTokString();
//    TestTokSpecial();
//    TestTokenizer();
//
//    TestRegisterPass();
//    TestRequiredPass();
//    TestPassManager();
//    TestVirtualTable();

//    TestSemanticCheckingPasses();

    TestFrontEnd();
}