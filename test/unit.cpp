//
// Created by 田地 on 2021/6/2.
//
#include <iostream>
#include <sstream>
#include "unit.h"
#include "../parser.h"
#include "../tokenizer.h"
#include "../pass.h"
#include "../analysis.h"
#include "../stable.h"
#include "../builtin.h"
#include "../attrs.h"

using namespace std;

using namespace cool::parser;
using namespace cool::repr;
using namespace cool::pass;
using namespace cool::ana;
using namespace cool::builtin;
using namespace attr;

void TestMatchMultiple() {
    Parser parser({
        {Token::ID, "test", "test"},
        {Token::TypeID, "test", "test"}
    });
    vector<Token::Type> types = {Token::ID, Token::TypeID};
    assert(parser.MatchMultiple(types));
}

void TestParseID() {
    Parser parser({
        {Token::ID, "test", "test"},
        {Token::ID, "test", "test"}
    });
    try {
        auto id = parser.ParseID();
        auto id1 = parser.ParseID();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseUnary() {
    Parser parser({
        {Token::kNot, "", ""},
        {Token::ID, "test", "test"}
    });
    try {
        auto id = parser.ParseNot();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseBinary() {
    Parser parser({
        {Token::kAdd, "", ""},
        {Token::ID, "test", "test"}
    });
    try {
        ID id;
        auto binary = parser.ParseAdd(make_shared<ID>(id));
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseNew() {
    Parser parser({
        {Token::kNew, "", ""},
        {Token::TypeID, "test", "test"}
    });
    try {
        auto newExpr = parser.ParseNew();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseCall() {
    Parser parser({
        // case 1
        {Token::ID, "", ""},
        {Token::kOpenParen, "test", "test"},
        {Token::kCloseParen, "test", "test"},
        // case 2
        {Token::ID, "", ""},
        {Token::kOpenParen, "test", "test"},
        {Token::ID, "", ""},
        {Token::kCloseParen, "test", "test"},
        // case 3
        {Token::ID, "", ""},
        {Token::kOpenParen, "test", "test"},
        {Token::ID, "", ""},
        {Token::kComma, "", ""},
        {Token::ID, "", ""},
        {Token::kCloseParen, "test", "test"}
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
    Parser parser({
        {Token::ID, "test", "test"},
        {Token::kAssignment, "", ""},
        {Token::ID, "test", "test"}
    });
    try {
        auto assign = parser.ParseAssign();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseCase() {
    Parser parser({
        // case 1
        {Token::kCase, "test", "test"},
        {Token::ID, "", ""},
        {Token::kOf, "test", "test"},
        {Token::ID, "test", "test"},
        {Token::kColon, "test", "test"},
        {Token::TypeID, "test", "test"},
        {Token::kEval, "test", "test"},
        {Token::ID, "test", "test"},
        {Token::kSemiColon, "test", "test"},
        {Token::kEsac, "test", "test"},
        // case 2
        {Token::kCase, "test", "test"},
        {Token::ID, "", ""},
        {Token::kOf, "test", "test"},
        {Token::ID, "test", "test"},
        {Token::kColon, "test", "test"},
        {Token::TypeID, "test", "test"},
        {Token::kEval, "test", "test"},
        {Token::ID, "test", "test"},
        {Token::kSemiColon, "test", "test"},
        {Token::ID, "test", "test"},
        {Token::kColon, "test", "test"},
        {Token::TypeID, "test", "test"},
        {Token::kEval, "test", "test"},
        {Token::ID, "test", "test"},
        {Token::kSemiColon, "test", "test"},
        {Token::kEsac, "test", "test"}
    });
    try {
        auto case1 = parser.ParseCase();
        auto case2 = parser.ParseCase();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseLet() {
    Parser parser({
        // case 1
        {Token::kLet, "", ""},
        {Token::ID, "test", "test"},
        {Token::kColon, "test", "test"},
        {Token::TypeID, "", ""},
        {Token::kIn, "test", "test"},
        {Token::ID, "", ""},
        // case 2
        {Token::kLet, "", ""},
        {Token::ID, "test", "test"},
        {Token::kColon, "test", "test"},
        {Token::TypeID, "", ""},
        {Token::kAssignment, "", ""},
        {Token::ID, "", ""},
        {Token::kIn, "test", "test"},
        {Token::ID, "", ""},
        // case 3
        {Token::kLet, "", ""},
        {Token::ID, "test", "test"},
        {Token::kColon, "test", "test"},
        {Token::TypeID, "", ""},
        {Token::kAssignment, "", ""},
        {Token::ID, "", ""},
        {Token::kComma, "", ""},
        {Token::ID, "test", "test"},
        {Token::kColon, "test", "test"},
        {Token::TypeID, "", ""},
        {Token::kAssignment, "", ""},
        {Token::ID, "", ""},
        {Token::kIn, "test", "test"},
        {Token::ID, "", ""},
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
    Parser parser({
        {Token::kWhile, "test", "test"},
        {Token::ID, "test", "test"},
        {Token::kLoop, "", ""},
        {Token::ID, "test", "test"},
        {Token::kPool, "test", "test"}
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
                {Token::kOpenBrace, "test", "test"},
                {Token::ID, "test", "test"},
                {Token::kSemiColon, "", ""},
                {Token::kCloseBrace, "test", "test"}
            },
            false,
        },
        {
            "case2",
            {
                {Token::kOpenBrace, "test", "test"},
                {Token::ID, "test", "test"},
                {Token::kSemiColon, "", ""},
                {Token::ID, "test", "test"},
                {Token::kSemiColon, "", ""},
                {Token::kCloseBrace, "test", "test"},
            },
            false,
        },
        {
            "case3",
            {
                {Token::kOpenBrace, "test", "test"},
                {Token::kCloseBrace, "test", "test"},
            },
            true,
        },
    };
    for (auto& tc : cases) {
        cout<< tc.name <<endl;
        try {
            Parser parser(tc.toks);
            auto block = parser.ParseBlock();
            if (tc.throwExp) assert(false);
        } catch (exception& e) {
            if (!tc.throwExp) assert(false);
        }
    }
}

void TestParseIf() {
    Parser parser({
        {Token::kIf, "test", "test"},
        {Token::ID, "test", "test"},
        {Token::kThen, "", ""},
        {Token::ID, "test", "test"},
        {Token::kElse, "test", "test"},
        {Token::ID, "test", "test"},
        {Token::kFi, "test", "test"},
    });
    try {
        auto ifExpr = parser.ParseIf();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseExpr() {
    Parser parser({
        {Token::ID, "test", "test"},
        {Token::kAdd, "", ""},
        {Token::ID, "test", "test"}
    });
    try {
        auto expr = parser.ParseExpr();
    } catch (exception& e) {
        assert(false);
    }
}

void TestParseFormal() {
    Parser parser({
        {Token::ID, "test", "test"},
        {Token::kColon, "", ""},
        {Token::TypeID, "test", "test"}
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
    Parser parser(toks);
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
    Parser parser(toks);
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
    Parser parser(toks);
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
    Parser parser(toks);
    try {
        parser.ParseProgram();
    } catch (exception& e) {
        assert(false);
    }
}

void TestTokDigit() {
    vector<TokComponentTestCase> cases = {
        {"0", {Token::Integer, "0", "0"}},
        {"00", {Token::Integer, "00", "00"}},
        {"01", {Token::Integer, "01", "01"}},
        {"123", {Token::Integer, "123", "123"}},
        {"12a", {Token::Integer, "12", "12"}},
    };
    for (auto& c : cases) {
        stringstream sstream;
        sstream<< c.str;
        Tokenizer tokenizer({});
        auto tok = tokenizer.TokDigit(sstream);
        assert(tok.type == c.tok.type && tok.str == c.tok.str && tok.val == c.tok.val);
    }
}

void TestTokAlpha() {
    vector<TokComponentTestCase> cases = {
        {"abc", {Token::ID, "abc", "abc"}},
        {"_abc", {Token::ID, "_abc", "_abc"}},
        {"ab1_1", {Token::ID, "ab1_1", "ab1_1"}},
        {"test space", {Token::ID, "test", "test"}},
        {"test\nspace", {Token::ID, "test", "test"}},
        {"File", {Token::TypeID, "File", "File"}},
        {"File inherits", {Token::TypeID, "File", "File"}},
        {"File_Reader1", {Token::TypeID, "File_Reader1", "File_Reader1"}},
        {"if", {Token::kIf, "", ""}},
        {"If", {Token::kIf, "", ""}},
        {"ifabc", {Token::ID, "ifabc", "ifabc"}},
    };
    for (auto& c : cases) {
        stringstream sstream;
        sstream<< c.str;
        Tokenizer tokenizer({});
        auto tok = tokenizer.TokAlpha(sstream);
        assert(tok.type == c.tok.type && tok.str == c.tok.str && tok.val == c.tok.val);
    }
}

void TestTokString() {
    vector<TokComponentTestCase> cases = {
        {"\"\"", {Token::String, "", ""}},
        {"\"abc\"", {Token::String, "abc", "abc"}},
        {"\"abc\"\"", {Token::String, "abc", "abc"}},
        {"\"\\\"\"", {Token::String, "\"", "\""}},
        {"\"\\\"\\c\"", {Token::String, "\"c", "\"c"}},
    };
    for (auto& c : cases) {
        stringstream sstream;
        sstream<< c.str;
        Tokenizer tokenizer({});
        auto tok = tokenizer.TokString(sstream);
        assert(tok.type == c.tok.type && tok.str == c.tok.str && tok.val == c.tok.val);
    }
}

void TestTokSpecial() {
    vector<TokComponentTestCase> cases = {
        {"<-", {Token::kAssignment, "", ""}},
        {"<6", {Token::kLessThan, "", ""}},
        {"<=6", {Token::kLessThanOrEqual, "", ""}},
        {"=6", {Token::kEqual, "", ""}},
        {"=>6", {Token::kEval, "", ""}},
        {";", {Token::kSemiColon, "", ""}},
    };
    for (auto& c : cases) {
        stringstream sstream;
        sstream<< c.str;
        Tokenizer tokenizer({});
        auto tok = tokenizer.TokSpecial(sstream);
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
         "};", {
            {Token::kClass, "", ""},
            {Token::TypeID, "B", "B"},
            {Token::kOpenBrace, "", ""},
            {Token::ID, "s", "s"},
            {Token::kColon, "", ""},
            {Token::TypeID, "String", "String"},
            {Token::kAssignment, "", ""},
            {Token::String, "Hello", "Hello"},
            {Token::kSemiColon, "", ""},
            {Token::ID, "g", "g"},
            {Token::kOpenParen, "", ""},
            {Token::ID, "y", "y"},
            {Token::kColon, "", ""},
            {Token::TypeID, "String", "String"},
            {Token::kCloseParen, "", ""},
            {Token::kColon, "", ""},
            {Token::TypeID, "String", "String"},
            {Token::kOpenBrace, "", ""},
            {Token::ID, "y", "y"},
            {Token::kDot, "", ""},
            {Token::ID, "concat", "concat"},
            {Token::kOpenParen, "", ""},
            {Token::ID, "s", "s"},
            {Token::kCloseParen, "", ""},
            {Token::kCloseBrace, "", ""},
            {Token::kSemiColon, "", ""},
            {Token::kCloseBrace, "", ""},
            {Token::kSemiColon, "", ""}
        }},
    };
    for (auto& c : cases) {
        stringstream sstream(c.str);
        Tokenizer tokenizer;
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

void TestTypeAttrs() {
    auto father = make_shared<TypeAttr>(TypeAttr{"f"});
    auto c1 = make_shared<TypeAttr>(TypeAttr{"c1", father});
    auto c2 = make_shared<TypeAttr>(TypeAttr{"c2", father});
    father->children.emplace_back(c1);
    father->children.emplace_back(c2);

    assert(c1->LeastCommonAncestor(c2).name == "f");
    assert(c1->LeastCommonAncestor(father).name == "f");
    assert(father->LeastCommonAncestor(c2).name == "f");
    assert(c1->LeastCommonAncestor(c1).name == "c1");

    assert(c1->Conforms("f"));
//    assert(c1->Conforms(*father));
}

void TestSemanticChecking() {
    vector<Token> toks;
    ConstructProgTokens(toks, false, testFuncFeats, testFieldFeats);
    Parser parser(toks);
    auto prog = parser.ParseProgram();
    int clsNum = prog.classes.size();
    PassContext passContext;
    try {
        // Test InstallBuiltin Pass
        InstallBuiltin installBuiltin;
        installBuiltin(prog, passContext);
        assert(prog.classes.size() == clsNum + BuiltinClassSet.size());

        // Test InitSymbolTable Pass
        InitSymbolTable initSymbolTable;
        initSymbolTable(prog, passContext);
        assert(passContext.Get<ScopedTable<SymbolTable>>("symbol_table"));

        // Test BuildInheritanceTree Pass
        BuildInheritanceTree buildInheritanceTree;
        buildInheritanceTree(prog, passContext);
        auto stable = passContext.Get<ScopedTable<SymbolTable>>("symbol_table");
        stable->InitTraverse();
        ENTER_SCOPE_GUARD((*stable), {
            for (auto& cls : prog.classes) {
                assert(!cls->parent.empty());
                auto typeAttr = stable->Current().GetTypeAttr(cls->name);
                assert(typeAttr->parent->name == "Object");
            }
        })
    } catch (exception& e) {
        cerr<< e.what() <<endl;
        assert(false);
    }
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

    TestTypeAttrs();

    TestSemanticChecking();
}