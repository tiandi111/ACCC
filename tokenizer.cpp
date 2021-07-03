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
using namespace diag;

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

unordered_map<Token::Type, string> tok::tokenStr = {
    {Token::ID, "identifier"},
    {Token::TypeID, "type identifier"},
    {Token::Integer, "Integer"},
    {Token::String, "String"},
    {Token::kClass, "class"},
    {Token::kIf, "if"},
    {Token::kThen, "then"},
    {Token::kElse, "else"},
    {Token::kFi, "fi"},
    {Token::kIn, "in"},
    {Token::kInheirits, "inherits"},
    {Token::kIsvoid, "isvoid"},
    {Token::kLet, "let"},
    {Token::kLoop, "loop"},
    {Token::kPool, "pool"},
    {Token::kWhile, "while"},
    {Token::kCase, "case"},
    {Token::kEsac, "esac"},
    {Token::kNew, "new"},
    {Token::kOf, "of"},
    {Token::kNot, "not"},
    {Token::kTrue, "true"},
    {Token::kFalse, "false"},
    {Token::kColon, ":"},
    {Token::kSemiColon, ";"},
    {Token::kComma, ","},
    {Token::kDot, "."},
    {Token::kNegate, "~"},
    {Token::kMultiply, "*"},
    {Token::kAdd, "+"},
    {Token::kMinus, "-"},
    {Token::kDivide, "/"},
    {Token::kOpenParen, "("},
    {Token::kCloseParen, ")"},
    {Token::kOpenBrace, "{"},
    {Token::kCloseBrace, "}"},
    {Token::kLessThanOrEqual, "<="},
    {Token::kLessThan, "<"},
    {Token::kEqual, "="},
};

Tokenizer::Tokenizer(diag::Diagnosis& _diag) : diag(_diag), line(1), pos(1) {}

Token Tokenizer::TokDigit(istream& in) {
    assert(isdigit(in.peek()));
    int stPos = pos;
    string str;
    while (isdigit(in.peek())) {
        str += char(in.get());
        pos++;
    }
    return Token(Token::Integer, str, str, line, stPos, fileno);
}

Token Tokenizer::TokAlpha(istream& in) {
    assert(isalpha(in.peek()) || in.peek() == '_');
    int stPos = pos;
    string str = {};
    while (isalpha(in.peek()) || isdigit(in.peek()) || in.peek() == '_') {
        str += char(in.get());
        pos++;
    }
    string lowerStr(str.begin(), str.end());
    bool capitalFirst = isupper(lowerStr.at(0));
    transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    if (keywordMap.find(lowerStr) != keywordMap.end()) {
        return Token(keywordMap.at(lowerStr), "", "", line, stPos, fileno);
    }
    if (capitalFirst) return Token(Token::TypeID, str, str, line, stPos, fileno);
    return Token(Token::ID, str, str, line, stPos, fileno);
}

Token Tokenizer::TokString(istream& in) {
    assert(in.peek() == '"');
    int stPos = pos;
    in.ignore(1);
    pos++;
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
        pos++;
    }
    return Token(Token::String, str, str, line, stPos, fileno);
}

Token Tokenizer::TokSpecial(istream& in) {
    int stPos = pos;
    char c = in.get();
    pos++;
    switch (c) {
        case '<':
            if (in.peek() == '-') {
                in.ignore(1);
                pos++;
                return Token(Token::kAssignment, "", "", line, stPos, fileno);
            }
            if (in.peek() == '=') {
                in.ignore(1);
                pos++;
                return Token(Token::kLessThanOrEqual, "", "", line, stPos, fileno);
            }
            return Token(Token::kLessThan, "", "", line, stPos, fileno);
        case '=':
            if (in.peek() == '>') {
                in.ignore(1);
                pos++;
                return Token(Token::kEval, "", "", line, stPos, fileno);
            }
            return Token(Token::kEqual, "", "", line, stPos, fileno);
        default:
            if (tokenMap.find(c) == tokenMap.end()) {
                diag.EmitError(line, 0, string("invalid character: ") += c);
                return Token(Token::SKIP, "", "", line, stPos, fileno);
            }
            return Token(tokenMap.at(c), "", "", line, stPos, fileno);
    }
}

vector<Token> Tokenizer::Tokenize(const string& file, istream& in) {
    fileno = FileMapper::GetFileNo(file);
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
            if (c == '\n') {
                line++;
                pos = 1;
            } else pos++;
            in.ignore(1);
        } else {
            auto tok = TokSpecial(in);
            if (!tok.Skip()) toks.emplace_back(tok);
        }

    }

    return toks;
}

