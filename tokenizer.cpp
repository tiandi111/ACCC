//
// Created by 田地 on 2021/6/3.
//

#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <string>

#include "cctype"

#include "token.h"
#include "tokenizer.h"

using namespace std;

unordered_map<string, Token> keywordMap = {
    {"class", Token{Token::kClass, "", ""}},
    {"if", Token{Token::kIf, "", ""}},
    {"then", Token{Token::kThen, "", ""}},
    {"else", Token{Token::kElse, "", ""}},
    {"fi", Token{Token::kFi, "", ""}},
    {"in", Token{Token::kIn, "", ""}},
    {"inherits", Token{Token::kInheirits, "", ""}},
    {"isvoid", Token{Token::kIsvoid, "", ""}},
    {"let", Token{Token::kLet, "", ""}},
    {"loop", Token{Token::kLoop, "", ""}},
    {"pool", Token{Token::kPool, "", ""}},
    {"while", Token{Token::kWhile, "", ""}},
    {"case", Token{Token::kCase, "", ""}},
    {"esac", Token{Token::kEsac, "", ""}},
    {"new", Token{Token::kNew, "", ""}},
    {"of", Token{Token::kOf, "", ""}},
    {"not", Token{Token::kNot, "", ""}},
    {"true", Token{Token::kTrue, "", ""}},
    {"false", Token{Token::kFalse, "", ""}},
};

unordered_map<char, Token> tokenMap = {
    {':', Token{Token::kColon, "", ""}},
    {';', Token{Token::kSemiColon, "", ""}},
    {',', Token{Token::kComma, "", ""}},
    {'.', Token{Token::kDot, "", ""}},
    {'~', Token{Token::kNegate, "", ""}},
    {'*', Token{Token::kMultiply, "", ""}},
    {'+', Token{Token::kAdd, "", ""}},
    {'-', Token{Token::kMinus, "", ""}},
    {'/', Token{Token::kDivide, "", ""}},
    {'(', Token{Token::kOpenParen, "", ""}},
    {')', Token{Token::kCloseParen, "", ""}},
    {'{', Token{Token::kOpenBrace, "", ""}},
    {'}', Token{Token::kCloseBrace, "", ""}},
};

Token Tokenizer::TokDigit(istream& in) {
    assert(isdigit(in.peek()));
    string str;
    while (isdigit(in.peek()))
        str += char(in.get());
    return Token(Token::Integer, str, str);
}

Token Tokenizer::TokAlpha(istream& in) {
    assert(isalpha(in.peek()) || in.peek() == '_');
    string str = {char(in.get())};
    while (isalpha(in.peek()) || isdigit(in.peek()) || in.peek() == '_') {
        str += char(in.get());
    }
    string lowerStr(str.begin(), str.end());
    bool capitalFirst = isupper(lowerStr.at(0));
    transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    if (keywordMap.find(lowerStr) != keywordMap.end()) {
        return keywordMap.at(lowerStr);
    }
    if (capitalFirst) return Token(Token::TypeID, str, str);
    return Token(Token::ID, str, str);
}

Token Tokenizer::TokString(istream& in) {
    assert(in.peek() == '"');
    in.ignore(1);
    string str;
    bool easc = false;
    while (in.good() && in.peek() != EOF) {
        if (easc) {
            str += char(in.get());
            easc = false;
            continue;
        }
        if (in.peek() == '\\') {
            in.ignore(1);
            easc = true;
        } else if (in.peek() == '"') {
            in.ignore(1);
            break;
        } else {
            str += char(in.get());
        }
    }
    return Token(Token::String, str, str);
}

Token Tokenizer::TokSpecial(istream& in) {
    char c = in.get();
    switch (c) {
        case '<':
            if (in.peek() == '-') {
                in.ignore(1);
                return Token(Token::kAssignment, "", "");
            }
            if (in.peek() == '=') {
                in.ignore(1);
                return Token(Token::kLessThanOrEqual, "", "");
            }
            return Token(Token::kLessThan, "", "");
        case '=':
            if (in.peek() == '>') {
                in.ignore(1);
                return Token(Token::kEval, "", "");
            }
            return Token(Token::kEqual, "", "");
        default:
            if (tokenMap.find(c) == tokenMap.end()) {
                throw runtime_error("invalid character");
            }
            return tokenMap.at(c);
    }
}

vector<Token> Tokenizer::Tokenize(istream& in) {
    vector<Token> toks;

    while (in.good() && in.peek() != EOF) {
        char c = in.peek();

        if (isdigit(c)) {
            toks.emplace_back(TokDigit(in));
        } else if (isalpha(c) || c == '_') {
            toks.emplace_back(TokAlpha(in));
        } else if (c == '"') {
            toks.emplace_back(TokString(in));
        } else if (isspace(c)) {
            in.ignore(1);
        } else {
            toks.emplace_back(TokSpecial(in));
        }

    }

    return toks;
}

