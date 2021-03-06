//
// Created by 田地 on 2021/6/2.
//
#include <iostream>
#include <sstream>
#include <fstream>

#include "unit.h"
#include "../frontend/parser.h"
#include "../frontend/tokenizer.h"
#include "../frontend/pass.h"
#include "../frontend/analysis.h"
#include "../frontend/builtin.h"

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

void TestProgram() {
    Program prog = {{}, {}};

    Class cls1 = {{"test"}, {"p1"}, {}, {}};

    assert(prog.AddClass(&cls1));
    assert(prog.GetClassPtr(cls1.GetName().Value()));
    prog.DeleteClass(cls1.GetName().Value());
    assert(!prog.GetClassPtr(cls1.GetName().Value()));
}

void TestMatchMultiple() {
    Parser parser(testDiag, {
        {Token::ID, "test", "test", 0, 0},
        {Token::TypeID, "test", "test", 0, 0}
    });
    vector<Token::Type> types = {Token::ID, Token::TypeID};
    assert(parser.MatchMultiple(types));
}

void TestParseID() {
    Parser parser(testDiag, {
        {Token::ID, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0}
    });
    try {
        auto id = parser.ParseID();
        auto id1 = parser.ParseID();
        testDiag.Output(cout);
        assert(testDiag.Size() == 0);
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseNew() {
    Parser parser(testDiag, {
        {Token::kNew, "", "", 0, 0},
        {Token::TypeID, "test", "test", 0, 0}
    });
    try {
        auto newExpr = parser.ParseNew();
        testDiag.Output(cout);
        assert(testDiag.Size() == 0);
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseCall() {
    Parser parser(testDiag, {
        // case 1
        {Token::ID, "test", "test", 0, 0},
        {Token::kOpenParen, "test", "test", 0, 0},
        {Token::kCloseParen, "test", "test", 0, 0},
        // case 2
        {Token::ID, "test", "test", 0, 0},
        {Token::kOpenParen, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kCloseParen, "test", "test", 0, 0},
        // case 3
        {Token::ID, "test", "test", 0, 0},
        {Token::kOpenParen, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kComma, "", "", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kCloseParen, "test", "test", 0, 0}
    });
    try {
        auto call1 = parser.ParseCall();
        auto call2 = parser.ParseCall();
        auto call3 = parser.ParseCall();
        testDiag.Output(cout);
        assert(testDiag.Size() == 0);
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseAssign() {
    Parser parser(testDiag, {
        {Token::ID, "test", "test", 0, 0},
        {Token::kAssignment, "", "", 0, 0},
        {Token::ID, "test", "test", 0, 0}
    });
    try {
        auto assign = parser.ParseAssign();
        testDiag.Output(cout);
        assert(testDiag.Size() == 0);
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseCase() {
    Parser parser(testDiag, {
        // case 1
        {Token::kCase, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kOf, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kColon, "test", "test", 0, 0},
        {Token::TypeID, "test", "test", 0, 0},
        {Token::kEval, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kSemiColon, "test", "test", 0, 0},
        {Token::kEsac, "test", "test", 0, 0},
        // case 2
        {Token::kCase, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kOf, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kColon, "test", "test", 0, 0},
        {Token::TypeID, "test", "test", 0, 0},
        {Token::kEval, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kSemiColon, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kColon, "test", "test", 0, 0},
        {Token::TypeID, "test", "test", 0, 0},
        {Token::kEval, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kSemiColon, "test", "test", 0, 0},
        {Token::kEsac, "test", "test", 0, 0}
    });
    try {
        auto case1 = parser.ParseCase();
        auto case2 = parser.ParseCase();
        testDiag.Output(cout);
        assert(testDiag.Size() == 0);
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseLet() {
    vector<vector<Token>> testCases = {
        {
            {Token::kLet, "", "", 0, 0},
            {Token::ID, "test", "test", 0, 0},
            {Token::kColon, "test", "test", 0, 0},
            {Token::TypeID, "test", "test", 0, 0},
            {Token::kIn, "test", "test", 0, 0},
            {Token::ID, "test", "test", 0, 0}
        },
        {
            {Token::kLet, "", "", 0, 0},
            {Token::ID, "test", "test", 0, 0},
            {Token::kColon, "test", "test", 0, 0},
            {Token::TypeID, "test", "test", 0, 0},
            {Token::kAssignment, "", "", 0, 0},
            {Token::ID, "test", "test", 0, 0},
            {Token::kIn, "test", "test", 0, 0},
            {Token::ID, "test", "test", 0, 0},
        },
        {
            {Token::kLet, "", "", 0, 0},
            {Token::ID, "test", "test", 0, 0},
            {Token::kColon, "test", "test", 0, 0},
            {Token::TypeID, "test", "test", 0, 0},
            {Token::kAssignment, "", "", 0, 0},
            {Token::ID, "test", "test", 0, 0},
            {Token::kComma, "", "", 0, 0},
            {Token::ID, "test", "test", 0, 0},
            {Token::kColon, "test", "test", 0, 0},
            {Token::TypeID, "test", "test", 0, 0},
            {Token::kAssignment, "", "", 0, 0},
            {Token::ID, "test", "test", 0, 0},
            {Token::kIn, "test", "test", 0, 0},
            {Token::ID, "test", "test", 0, 0},
        }};
    for (auto& tc : testCases) {
        Parser parser(testDiag, tc);
        try {
            parser.ParseLet();
            testDiag.Output(cout);
            assert(testDiag.Size() == 0);
        } catch (exception& e) {
            assert(false);
        }
    }
}

void TestParseWhile() {
    Parser parser(testDiag, {
        {Token::kWhile, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kLoop, "", "", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kPool, "test", "test", 0, 0}
    });
    try {
        auto whileExpr = parser.ParseWhile();
        testDiag.Output(cout);
        assert(testDiag.Size() == 0);
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseBlock() {
    vector<ParserTestCase> cases = {
        {
            "case1",
            {
                {Token::kOpenBrace, "test", "test", 0, 0},
                {Token::ID, "test", "test", 0, 0},
                {Token::kSemiColon, "", "", 0, 0},
                {Token::kCloseBrace, "test", "test", 0, 0}
            },
            false,
        },
        {
            "case2",
            {
                {Token::kOpenBrace, "test", "test", 0, 0},
                {Token::String, "test", "test", 0, 0},
                {Token::kSemiColon, "", "", 0, 0},
                {Token::ID, "test", "test", 0, 0},
                {Token::kSemiColon, "", "", 0, 0},
                {Token::kCloseBrace, "test", "test", 0, 0},
            },
            false,
        },
        {
            "case3",
            {
                {Token::kOpenBrace, "test", "test", 0, 0},
                {Token::kCloseBrace, "test", "test", 0, 0},
            },
            true,
        },
    };
    for (auto& tc : cases) {
        diag::Diagnosis diag;
        Parser parser(diag, tc.toks);
        auto block = parser.ParseBlock();
        diag.Output(cout);
        assert(tc.emitError == (diag.Size() > 0));
    }
}

void TestParseIf() {
    diag::Diagnosis diag;
    Parser parser(diag, {
        {Token::kIf, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kThen, "", "", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kElse, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kFi, "test", "test", 0, 0},
    });
    try {
        auto ifExpr = parser.ParseIf();
        diag.Output(cout);
        assert(diag.Size() == 0);
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseExpr() {
    diag::Diagnosis diag;
    Parser parser(diag, {
        {Token::ID, "test", "test", 0, 0},
        {Token::kAdd, "", "", 0, 0},
        {Token::ID, "test", "test", 0, 0}
    });
    try {
        auto expr = parser.ParseExpr();
        diag.Output(cout);
        assert(diag.Size() == 0);
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseFormal() {
    diag::Diagnosis diag;
    Parser parser(diag, {
        {Token::ID, "test", "test", 0, 0},
        {Token::kColon, "", "", 0, 0},
        {Token::TypeID, "test", "test", 0, 0}
    });
    try {
        auto formal = parser.ParseFormal();
        diag.Output(cout);
        assert(diag.Size() == 0);
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseFuncFeature() {
    for (auto& toks : testFuncFeats) {
        diag::Diagnosis diag;
        Parser parser(diag, toks);
        try {
            parser.ParseFuncFeature();
            diag.Output(cout);
            assert(diag.Size() == 0);
        } catch (exception& e) {
            cout<< e.what() <<endl;
            assert(false);
        }
    }
}

void TestParseFieldFeature() {
    diag::Diagnosis diag;
    vector<Token> toks;
    for (auto& featureToks : testFieldFeats) {
        for (auto& tok : featureToks) {
            toks.emplace_back(tok);
        }
    }
    Parser parser(diag, toks);
    try {
        for (int i = 0; i < testFieldFeats.size(); i++) {
            parser.ParseFieldFeature();
            diag.Output(cout);
            assert(diag.Size() == 0);
        }
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseClass() {
    diag::Diagnosis diag;
    vector<Token> toks;
    ConstructClassTokens(toks, true, testFuncFeats, testFieldFeats);
    ConstructClassTokens(toks, false, testFuncFeats, testFieldFeats);
    Parser parser(diag, toks);
    try {
        parser.ParseClass();
        parser.ParseClass();
        diag.Output(cout);
        assert(diag.Size() == 0);
    } catch (exception& e) {
        cerr<< e.what() <<endl;
        assert(false);
    }
}

void TestParseProgram() {
    diag::Diagnosis diag;
    vector<Token> toks;
    ConstructClassTokens(toks, true, testFuncFeats, testFieldFeats);
    try {
        Parser parser(diag, toks);
        parser.ParseProgram();
        diag.Output(cout);
        assert(diag.Size() == 0);
    } catch (exception& e) {
        assert(false);
    }
}

void TestTokDigit() {
    vector<TokComponentTestCase> cases = {
        {"0", {Token::Integer, "0", "0", 0, 0}},
        {"00", {Token::Integer, "00", "00", 0, 0}},
        {"01", {Token::Integer, "01", "01", 0, 0}},
        {"123", {Token::Integer, "123", "123", 0, 0}},
        {"12a", {Token::Integer, "12", "12", 0, 0}},
    };
    for (auto& c : cases) {
        stringstream sstream;
        sstream<< c.str;
        Tokenizer tokenizer(testDiag);
        auto tok = tokenizer.TokDigit(sstream);
        assert(tok.type == c.tok.type && tok.str == c.tok.str && tok.val == c.tok.val);
    }
}

void TestTokAlpha() {
    vector<TokComponentTestCase> cases = {
        {"abc", {Token::ID, "abc", "abc", 0, 0}},
        {"_abc", {Token::ID, "_abc", "_abc", 0, 0}},
        {"ab1_1", {Token::ID, "ab1_1", "ab1_1", 0, 0}},
        {"test space", {Token::ID, "test", "test", 0, 0}},
        {"test\nspace", {Token::ID, "test", "test", 0, 0}},
        {"File", {Token::TypeID, "File", "File", 0, 0}},
        {"File inherits", {Token::TypeID, "File", "File", 0, 0}},
        {"File_Reader1", {Token::TypeID, "File_Reader1", "File_Reader1", 0, 0}},
        {"if", {Token::kIf, "", "", 0, 0}},
        {"If", {Token::kIf, "", "", 0, 0}},
        {"ifabc", {Token::ID, "ifabc", "ifabc", 0, 0}},
    };
    for (auto& c : cases) {
        stringstream sstream;
        sstream<< c.str;
        Tokenizer tokenizer(testDiag);
        auto tok = tokenizer.TokAlpha(sstream);
        assert(tok.type == c.tok.type && tok.str == c.tok.str && tok.val == c.tok.val);
    }
}

void TestTokString() {
    vector<TokComponentTestCase> cases = {
        {"\"\"", {Token::String, "", "", 0, 0}},
        {"\"abc\"", {Token::String, "abc", "abc", 0, 0}},
        {"\"abc\"\"", {Token::String, "abc", "abc", 0, 0}},
        {"\"\\\"\"", {Token::String, "\"", "\"", 0, 0}},
        {"\"\\\"\\c\"", {Token::String, "\"c", "\"c", 0, 0}},
    };
    for (auto& c : cases) {
        stringstream sstream;
        sstream<< c.str;
        Tokenizer tokenizer(testDiag);
        auto tok = tokenizer.TokString(sstream);
        assert(tok.type == c.tok.type && tok.str == c.tok.str && tok.val == c.tok.val);
    }
}

void TestTokSpecial() {
    vector<TokComponentTestCase> cases = {
        {"<-", {Token::kAssignment, "", "", 0, 0}},
        {"<6", {Token::kLessThan, "", "", 0, 0}},
        {"<=6", {Token::kLessThanOrEqual, "", "", 0, 0}},
        {"=6", {Token::kEqual, "", "", 0, 0}},
        {"=>6", {Token::kEval, "", "", 0, 0}},
        {";", {Token::kSemiColon, "", "", 0, 0}},
    };
    for (auto& c : cases) {
        stringstream sstream;
        sstream<< c.str;
        Tokenizer tokenizer(testDiag);
        auto tok = tokenizer.TokSpecial(sstream);
        assert(tok.type == c.tok.type && tok.str == c.tok.str && tok.val == c.tok.val);
    }
}

void TestTokComment() {
    Diagnosis diag;
    vector<TokComponentTestCase> cases = {
        {"--abc\n", {Token::Comment, "abc", "", 0, 0},
         false},
        {"-abc\n", {Token::SKIP, "abc", "", 0, 0},
         true},
        {"*abc*", {Token::Comment, "abc", "", 0, 0},
            false},
        {"*ab*c*", {Token::Comment, "ab", "", 0, 0},
            false},
    };
    for (auto& c : cases) {
        stringstream sstream;
        sstream<< c.str;
        Tokenizer tokenizer(diag);
        auto tok = tokenizer.TokComment(sstream);
        assert(tok.type == c.tok.type
        && tok.str == c.tok.str
        && tok.val == c.tok.val
        && (c.emitDiag == !diag.Empty())
        );
    }
}

void TestTokenizer() {
    vector<TokenizerTestCase> cases = {
        {"class B {\n"
         "    s : String <- \"Hello\";\n"
         "    g (y : String) : String {\n"
         "        y.concat(s)\n"
         "    };"
         "};", {
            {Token::kClass, "", "", 0, 0},
            {Token::TypeID, "B", "B", 0, 0},
            {Token::kOpenBrace, "", "", 0, 0},
            {Token::ID, "s", "s", 0, 0},
            {Token::kColon, "", "", 0, 0},
            {Token::TypeID, "String", "String", 0, 0},
            {Token::kAssignment, "", "", 0, 0},
            {Token::String, "Hello", "Hello", 0, 0},
            {Token::kSemiColon, "", "", 0, 0},
            {Token::ID, "g", "g", 0, 0},
            {Token::kOpenParen, "", "", 0, 0},
            {Token::ID, "y", "y", 0, 0},
            {Token::kColon, "", "", 0, 0},
            {Token::TypeID, "String", "String", 0, 0},
            {Token::kCloseParen, "", "", 0, 0},
            {Token::kColon, "", "", 0, 0},
            {Token::TypeID, "String", "String", 0, 0},
            {Token::kOpenBrace, "", "", 0, 0},
            {Token::ID, "y", "y", 0, 0},
            {Token::kDot, "", "", 0, 0},
            {Token::ID, "concat", "concat", 0, 0},
            {Token::kOpenParen, "", "", 0, 0},
            {Token::ID, "s", "s", 0, 0},
            {Token::kCloseParen, "", "", 0, 0},
            {Token::kCloseBrace, "", "", 0, 0},
            {Token::kSemiColon, "", "", 0, 0},
            {Token::kCloseBrace, "", "", 0, 0},
            {Token::kSemiColon, "", "", 0, 0}
        }},
    };
    for (auto& c : cases) {
        stringstream sstream(c.str);
        Tokenizer tokenizer(testDiag);
        auto toks = tokenizer.Tokenize("", sstream);
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
    auto prog = new Program();
    PassContext ctx(testDiag);
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
        PassManager::Run(prog, ctx);
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
            PassManager::Run(prog, ctx);
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

void TestFrontEnd() {
    string filename = "../test/data/test_program";
    fstream file;
    file.open(filename, ios::in);
    assert(file);
    Diagnosis diagnosis;
    Tokenizer tokenizer(diagnosis);
    Parser parser(diagnosis, tokenizer.Tokenize(filename, file));
    auto prog = parser.ParseProgram();
    PassContext passContext(diagnosis);
    pass::PassManager::Register<ana::InstallBuiltin>();
    pass::PassManager::Register<ana::InitSymbolTable>();
    pass::PassManager::Register<ana::BuildInheritanceTree>();
    pass::PassManager::Register<ana::TypeChecking>();
    pass::PassManager::Run(prog, passContext);
    diagnosis.Output(cout);
}


int main() {
    TestProgram();

    TestMatchMultiple();

    TestParseID();
    TestParseNew();
    TestParseCall();
    TestParseAssign();
    TestParseCase();
    TestParseLet();
    TestParseWhile();
    TestParseBlock();
    TestParseIf();

    TestParseExpr();
    TestParseFormal();
    TestParseFuncFeature();
    TestParseFieldFeature();
    TestParseClass();
    TestParseProgram();

    TestTokDigit();
    TestTokAlpha();
    TestTokString();
    TestTokSpecial();
    TestTokenizer();
    TestTokComment();

    TestRegisterPass();
    TestRequiredPass();
    TestPassManager();
    TestVirtualTable();

//    TestFrontEnd();
}