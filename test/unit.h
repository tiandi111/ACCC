//
// Created by 田地 on 2021/6/2.
//

#ifndef COOL_UNIT_H
#define COOL_UNIT_H

#include <string>
#include <vector>

#include "../token.h"
#include "../vtable.h"

using namespace std;

vector<vector<Token>> testFieldFeats = {
    {
        {Token::ID, "test", "test"},
        {Token::kColon, "", ""},
        {Token::TypeID, "test", "test"},
    },
    {
        {Token::ID, "test", "test"},
        {Token::kColon, "", ""},
        {Token::TypeID, "test", "test"},
        {Token::kAssignment, "test", "test"},
        {Token::ID, "test", "test"},
    }
};

vector<vector<Token>> testFuncFeats = {
    {
        {Token::ID, "test", "test"},
        {Token::kOpenParen, "", ""},
        {Token::kCloseParen, "test", "test"},
        {Token::kColon, "test", "test"},
        {Token::TypeID, "test", "test"},
        {Token::kOpenBrace, "test", "test"},
        {Token::ID, "test", "test"},
        {Token::kCloseBrace, "test", "test"},
    },
    {
        {Token::ID, "test", "test"},
        {Token::kOpenParen, "", ""},
        {Token::ID, "test", "test"},
        {Token::kColon, "", ""},
        {Token::TypeID, "test", "test"},
        {Token::kCloseParen, "test", "test"},
        {Token::kColon, "test", "test"},
        {Token::TypeID, "test", "test"},
        {Token::kOpenBrace, "test", "test"},
        {Token::ID, "test", "test"},
        {Token::kCloseBrace, "test", "test"},
    },
    {
        {Token::ID, "test", "test"},
        {Token::kOpenParen, "", ""},
        {Token::ID, "test", "test"},
        {Token::kColon, "", ""},
        {Token::TypeID, "test", "test"},
        {Token::kComma, "test", "test"},
        {Token::ID, "test", "test"},
        {Token::kColon, "", ""},
        {Token::TypeID, "test", "test"},
        {Token::kCloseParen, "test", "test"},
        {Token::kColon, "test", "test"},
        {Token::TypeID, "test", "test"},
        {Token::kOpenBrace, "test", "test"},
        {Token::kIsvoid, "test", "test"},
        {Token::ID, "test", "test"},
        {Token::kCloseBrace, "test", "test"},
    }
};

void ConstructClassTokens(vector<Token>& dest, bool inherits,
    vector<vector<Token>>& funcFeats, vector<vector<Token>>& fieldFeats) {
    dest.emplace_back(Token{Token::kClass, "test", "test"});
    dest.emplace_back(Token{Token::TypeID, "test", "test"});
    if (inherits) {
        dest.emplace_back(Token{Token::kInheirits, "test", "test"});
        dest.emplace_back(Token{Token::TypeID, "test", "test"});
    }
    dest.emplace_back(Token{Token::kOpenBrace, "test", "test"});
    for (auto& toks : funcFeats) {
        for (auto& tok : toks) {
            dest.emplace_back(tok);
        }
        dest.emplace_back(Token{Token::kSemiColon, "test", "test"});
    }
    for (auto& toks : fieldFeats) {
        for (auto& tok : toks) {
            dest.emplace_back(tok);
        }
        dest.emplace_back(Token{Token::kSemiColon, "test", "test"});
    }
    dest.emplace_back(Token{Token::kCloseBrace, "test", "test"});
}

void ConstructProgTokens(vector<Token>& dest, bool inherits,
    vector<vector<Token>>& funcFeats, vector<vector<Token>>& fieldFeats) {
    ConstructClassTokens(dest, inherits, funcFeats, fieldFeats);
    dest.emplace_back(Token{Token::kSemiColon, "test", "test"});
}

void TestMatchMultiple();

struct ParserTestCase {
    string name;
    vector<Token> toks;
    bool throwExp;
};

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

void TestTypeAttrs();

void TestSemanticChecking();

#endif //COOL_UNIT_H
