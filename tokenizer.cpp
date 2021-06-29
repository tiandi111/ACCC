//
// Created by 田地 on 2021/6/3.
//

#include <algorithm>
#include <unordered_map>

#include "cctype"

#include "token.h"
#include "tokenizer.h"

using namespace std;
using namespace cool;
using namespace tok;

unordered_map<string, Token::Type> tok::keywordMap = {
    {"class", Token::kClass},
    {"if", Token::kIf},
    {"then", Token::kThen},
    {"else", Token::kElse},
    {"fi", Token::kFi},
    {"in", Token::kIn},
    {"inherits", Token::kInheirits},
    {"isvoid", Token::kIsvoid},
    {"let", Token::kLet},
    {"loop", Token::kLoop},
    {"pool", Token::kPool},
    {"while", Token::kWhile},
    {"case", Token::kCase},
    {"esac", Token::kEsac},
    {"new", Token::kNew},
    {"of", Token::kOf},
    {"not", Token::kNot},
    {"true", Token::kTrue},
    {"false", Token::kFalse},
};

unordered_map<char, Token::Type> tok::tokenMap = {
    {':', Token::kColon},
    {';', Token::kSemiColon},
    {',', Token::kComma},
    {'.', Token::kDot},
    {'~', Token::kNegate},
    {'*', Token::kMultiply},
    {'+', Token::kAdd},
    {'-', Token::kMinus},
    {'/', Token::kDivide},
    {'(', Token::kOpenParen},
    {')', Token::kCloseParen},
    {'{', Token::kOpenBrace},
    {'}', Token::kCloseBrace},
};

Tokenizer::Tokenizer(diag::Diagnosis& _diag) : diag(_diag) {}

Token Tokenizer::TokDigit(int line, istream& in) {
    assert(isdigit(in.peek()));
    string str;
    while (isdigit(in.peek())) str += char(in.get());
    return Token(Token::Integer, str, str, line);
}

Token Tokenizer::TokAlpha(int line, istream& in) {
    assert(isalpha(in.peek()) || in.peek() == '_');
    string str = {char(in.get())};
    while (isalpha(in.peek()) || isdigit(in.peek()) || in.peek() == '_') str += char(in.get());
    string lowerStr(str.begin(), str.end());
    bool capitalFirst = isupper(lowerStr.at(0));
    transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    if (keywordMap.find(lowerStr) != keywordMap.end()) {
        return Token(keywordMap.at(lowerStr), "", "", line);
    }
    if (capitalFirst) return Token(Token::TypeID, str, str, line);
    return Token(Token::ID, str, str, line);
}

Token Tokenizer::TokString(int line, istream& in) {
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
    return Token(Token::String, str, str, line);
}

Token Tokenizer::TokSpecial(int line, istream& in) {
    char c = in.get();
    switch (c) {
        case '<':
            if (in.peek() == '-') {
                in.ignore(1);
                return Token(Token::kAssignment, "", "", line);
            }
            if (in.peek() == '=') {
                in.ignore(1);
                return Token(Token::kLessThanOrEqual, "", "", line);
            }
            return Token(Token::kLessThan, "", "", line);
        case '=':
            if (in.peek() == '>') {
                in.ignore(1);
                return Token(Token::kEval, "", "", line);
            }
            return Token(Token::kEqual, "", "", line);
        default:
            if (tokenMap.find(c) == tokenMap.end()) {
                diag.Emit(line, "invalid character: " + c);
                return Token(Token::SKIP, "", "", line);
            }
            return Token(tokenMap.at(c), "", "", line);
    }
}

vector<Token> Tokenizer::Tokenize(istream& in) {
    vector<Token> toks;
    int line = 1;

    while (in.good() && in.peek() != EOF) {
        char c = in.peek();

        if (isdigit(c)) {
            toks.emplace_back(TokDigit(line, in));
        } else if (isalpha(c) || c == '_') {
            toks.emplace_back(TokAlpha(line, in));
        } else if (c == '"') {
            toks.emplace_back(TokString(line, in));
        } else if (isspace(c)) {
            if (c == '\n') line++;
            in.ignore(1);
        } else {
            auto tok = TokSpecial(line, in);
            if (!tok.Skip()) toks.emplace_back(tok);
        }

    }

    return toks;
}

