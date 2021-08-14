//
// Created by 田地 on 2021/5/30.
//

#ifndef COOL_TOKEN_H
#define COOL_TOKEN_H

#include <string>
#include <utility>
#include <unordered_map>

#include "diag.h"

using namespace std;

namespace cool {

namespace tok {

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
    diag::TextInfo textInfo;

    Token(Type _type, string _str, string _val, int _line, int _pos, int _fileno = -1);

    static bool IsOperator(Type type) {
        return
            (type > kBinaryST && type < kBinaryEND) ||
            type == kNegate ||
            type == kNot ||
            type == kIsvoid;
    }

    static bool IsUnary(Type type) {
        return
            type == kNegate ||
            type == kNot ||
            type == kNew ||
            type == kIsvoid;
    }

    enum Precedence {
        Not,
        Comparator,
        Add,
        Multiply,
        IsVoid,
        Negate,
        Dot
    };

    static int GetOperatorPrecedence(Type type) {
        if (!IsOperator(type))
            throw runtime_error("not an operator expression");
        switch (type) {
            case kNot:
                return Not;
            case kLessThan:
            case kLessThanOrEqual:
            case kEqual:
                return Comparator;
            case kAdd:
            case kMinus:
                return Add;
            case kMultiply:
            case kDivide:
                return Multiply;
            case kIsvoid:
                return IsVoid;
            case kNegate:
                return Negate;
            case kDot:
                return Dot;
            default:
                assert(false);
        }
    }

    bool IsOperator();

    bool IsUnary();

    bool Skip();
};

} // namespace tok

} // namespace cool


#endif //COOL_TOKEN_H
