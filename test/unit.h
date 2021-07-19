//
// Created by 田地 on 2021/6/2.
//

#ifndef COOL_UNIT_H
#define COOL_UNIT_H

#include <string>
#include <vector>

#include "../frontend/token.h"
#include "../frontend/vtable.h"

using namespace std;
using namespace cool::tok;

vector<vector<Token>> testFieldFeats = {
    {
        {Token::ID, "test1", "test", 0, 0},
        {Token::kColon, "", "", 0, 0},
        {Token::TypeID, "test", "test", 0, 0},
        {Token::kSemiColon, "test", "test", 0, 0}
    },
    {
        {Token::ID, "test2", "test", 0, 0},
        {Token::kColon, "", "", 0, 0},
        {Token::TypeID, "test", "test", 0, 0},
        {Token::kAssignment, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kSemiColon, "test", "test", 0, 0}
    }
};

vector<vector<Token>> testFuncFeats = {
    {
        {Token::ID, "test1", "test", 0, 0},
        {Token::kOpenParen, "", "", 0, 0},
        {Token::kCloseParen, "test", "test", 0, 0},
        {Token::kColon, "test", "test", 0, 0},
        {Token::TypeID, "test", "test", 0, 0},
        {Token::kOpenBrace, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kCloseBrace, "test", "test", 0, 0},
        {Token::kSemiColon, "test", "test", 0, 0}
    },
    {
        {Token::ID, "test2", "test", 0, 0},
        {Token::kOpenParen, "", "", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kColon, "", "", 0, 0},
        {Token::TypeID, "test", "test", 0, 0},
        {Token::kCloseParen, "test", "test", 0, 0},
        {Token::kColon, "test", "test", 0, 0},
        {Token::TypeID, "test", "test", 0, 0},
        {Token::kOpenBrace, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kCloseBrace, "test", "test", 0, 0},
        {Token::kSemiColon, "test", "test", 0, 0}
    },
    {
        {Token::ID, "test3", "test", 0, 0},
        {Token::kOpenParen, "", "", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kColon, "", "", 0, 0},
        {Token::TypeID, "test", "test", 0, 0},
        {Token::kComma, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kColon, "", "", 0, 0},
        {Token::TypeID, "test", "test", 0, 0},
        {Token::kCloseParen, "test", "test", 0, 0},
        {Token::kColon, "test", "test", 0, 0},
        {Token::TypeID, "test", "test", 0, 0},
        {Token::kOpenBrace, "test", "test", 0, 0},
        {Token::kIsvoid, "test", "test", 0, 0},
        {Token::ID, "test", "test", 0, 0},
        {Token::kCloseBrace, "test", "test", 0, 0},
        {Token::kSemiColon, "test", "test", 0, 0}
    }
};

void ConstructClassTokens(vector<Token>& dest, bool inherits,
    vector<vector<Token>>& funcFeats, vector<vector<Token>>& fieldFeats) {
    dest.emplace_back(Token{Token::kClass, "test", "test", 0, 0});
    dest.emplace_back(Token{Token::TypeID, "test", "test", 0, 0});
    if (inherits) {
        dest.emplace_back(Token{Token::kInheirits, "", "", 0, 0});
        dest.emplace_back(Token{Token::TypeID, "Object", "Object", 0, 0});
    }
    dest.emplace_back(Token{Token::kOpenBrace, "test", "test", 0, 0});
    for (auto& toks : funcFeats) {
        for (auto& tok : toks) {
            dest.emplace_back(tok);
        }
    }
    for (auto& toks : fieldFeats) {
        for (auto& tok : toks) {
            dest.emplace_back(tok);
        }
    }
    dest.emplace_back(Token{Token::kCloseBrace, "test", "test", 0, 0});
    dest.emplace_back(Token{Token::kSemiColon, "test", "test", 0, 0});
}

void TestMatchMultiple();

struct ParserTestCase {
    string name;
    vector<Token> toks;
    bool throwExp;
};

void TestProgram();

void TestParseID();
void TestParseUnary();
void TestParseBinary();
void TestParseNew();
void TestParseCall();
void TestParseAssign();
void TestParseCase();
void TestParseLet();
void TestParseWhile();
void TestParseBlock();
void TestParseIf();
void TestParseExpr();
void TestParseFormal();
void TestParseFuncFeature();
void TestParseFieldFeature();
void TestParseClass();
void TestParseProgram();

struct TokComponentTestCase {
    string str;
    Token tok;
};

void TestTokDigit();
void TestTokAlpha();
void TestTokString();
void TestTokSpecial();

struct TokenizerTestCase {
    string str;
    vector<Token> toks;
};

void TestTokenizer();

void TestRegisterPass();
void TestRequiredPass();
void TestPassManager();

void TestVirtualTable();

void TestSemanticCheckingPasses();

void TestFrontEnd();

#endif //COOL_UNIT_H
