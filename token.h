//
// Created by 田地 on 2021/5/30.
//

#ifndef COOL_TOKEN_H
#define COOL_TOKEN_H

#include <string>
#include <utility>

using namespace std;

struct Token {
    enum Type {
        ST,
        ID,
        TypeID,
        Integer,
        String,
        kTrue,
        kFalse,
        kIf,
        kThen,
        kElse,
        kFi,
        kWhile,
        kLoop,
        kPool,
        kLet,
        kIn,
        kCase,
        kOf,
        kEsac,
        kNew,
        kIsvoid,
        kNot,
        kBinaryST,
        kAdd,
        kMinus,
        kMultiply,
        kDivide,
        kLessThan,
        kLessThanOrEqual,
        kEqual,
        kDot,
        kBinaryEND,
        kNegate,
        kClass,
        kInheirits,
        kOpenParen,
        kCloseParen,
        kSemiColon,
        kColon,
        kComma,
        kOpenBrace,
        kCloseBrace,
        kAssignment,
        kEval,
        SKIP, // skipped token
        END
    };
    Type type;
    string str;
    string val;
    int line = 0;

    Token(Type _type, string _str, string _val, int _line)
    : type(_type), str(std::move(_str)), val(std::move(_val)), line(_line) {
        assert(type > ST && type < END);
    }

    bool Skip() { return type == SKIP; }

};

#endif //COOL_TOKEN_H
