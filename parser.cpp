//
// Created by 田地 on 2021/5/30.
//

#include <iostream>
#include <stdexcept>
#include <vector>
#include <stack>

#include "parser.h"
#include "repr.h"

using namespace cool;
using namespace parser;
using namespace repr;


void Parser::Consume() {
    pos++;
}

Token Parser::ConsumeReturn() {
    auto& tok = toks.at(pos);
    Consume();
    return tok;
}

bool Parser::ConsumeIfMatch(Token::Type type) {
    if (Match(type)) {
        Consume();
        return true;
    }
    return false;
}

bool Parser::Match(Token::Type type) {
    return !toks.empty() && toks[pos].type == type;
}

bool Parser::MatchMultiple(vector<Token::Type> types) {
    if ((toks.size() - pos) < types.size()) return false;
    for (int i = pos; i < pos + types.size(); i++) {
        if (types.at(i-pos) != toks.at(i).type) return false;
    }
    return true;
}

template<typename T>
vector<T> Parser::MatchSequence(Token::Type start, Token::Type sep, Token::Type end, function<T()> f) {
    if (!ConsumeIfMatch(start)) {
        throw runtime_error("");
    }

    vector<T> items;

    while (true) {
        if (ConsumeIfMatch(end)) {
            break;
        }
        if (!items.empty() && !ConsumeIfMatch(sep)) {
            throw runtime_error("");
        }
        items.emplace_back(f());
    }

    return items;
}

Token& Parser::Peek() {
    return toks[pos];
}

int Parser::Empty() {
    return pos >= toks.size();
}

Program Parser::ParseProgram() {
    Program prog;

    while (!Empty()) {

        if (Match(Token::kClass)) {
            prog.classes.emplace_back(make_shared<Class>(ParseClass()));
        } else {
            throw runtime_error("");
        }

        ConsumeIfMatchWithException(Token::kSemiColon, "")
    }

    if (prog.classes.empty()) {
        throw runtime_error("");
    }

    return prog;
}

Class Parser::ParseClass() {
    ConsumeIfMatchWithException(Token::kClass, "expected class keyword")

    Class cls;

    AssignIfMatchWithException(Token::TypeID, cls.name = ConsumeReturn().str,
        "expected class identifier")

    if (ConsumeIfMatch(Token::kInheirits)) {
        AssignIfMatchWithException(Token::TypeID, cls.parent = ConsumeReturn().str,
            "expected class identifier")
    }

    ConsumeIfMatchWithException(Token::kOpenBrace, "expected {")

    while (!ConsumeIfMatch(Token::kCloseBrace)) {

        if (MatchMultiple({Token::ID, Token::kOpenParen})) {
            cls.funcs.emplace_back(make_shared<FuncFeature>(ParseFuncFeature()));
        } else {
            cls.fields.emplace_back(make_shared<FieldFeature>(ParseFieldFeature()));
        }

        ConsumeIfMatchWithException(Token::kSemiColon, "expected ;")
    }
    return cls;
}

FuncFeature Parser::ParseFuncFeature() {
    FuncFeature feat;

    AssignIfMatchWithException(Token::ID, feat.name = ConsumeReturn().str, "expected method identifier")
    feat.args = MatchSequence<shared_ptr<Formal>>(
        Token::kOpenParen,
        Token::kComma,
        Token::kCloseParen,
        [&]() { return make_shared<Formal>(ParseFormal()); });
    ConsumeIfMatchWithException(Token::kColon, "expected :");
    AssignIfMatchWithException(Token::TypeID, feat.type = ConsumeReturn().str, "expected type")
    ConsumeIfMatchWithException(Token::kOpenBrace, "expected {")
    feat.expr = ParseExpr();
    ConsumeIfMatchWithException(Token::kCloseBrace, "expected }")

    return feat;
}

FieldFeature Parser::ParseFieldFeature() {
    FieldFeature feat;

    AssignIfMatchWithException(Token::ID, feat.name = ConsumeReturn().str, "expected field identifier")
    ConsumeIfMatchWithException(Token::kColon, "expected :");
    AssignIfMatchWithException(Token::TypeID, feat.type = ConsumeReturn().str, "expected type identifier")
    if (ConsumeIfMatch(Token::kAssignment)) {
        feat.expr = ParseExpr();
    }
    return feat;
}

Formal Parser::ParseFormal() {
    Formal form;
    AssignIfMatchWithException(Token::ID, form.name = ConsumeReturn().str, "")
    ConsumeIfMatchWithException(Token::kColon, "")
    AssignIfMatchWithException(Token::TypeID, form.type = ConsumeReturn().str, "")
    return form;
}

shared_ptr<Expr> Parser::ParseExpr() {
    stack<shared_ptr<Expr>> exprStack;

    while (true) {
        shared_ptr<Expr> expr;
        auto tokType = Peek().type;
        if (exprStack.empty()) {
            switch (tokType) {
                case Token::kIf:
                    expr = make_shared<If>(ParseIf());
                    break;
                case Token::kWhile:
                    expr = make_shared<While>(ParseWhile());
                    break;
                case Token::kOpenBrace:
                    expr = make_shared<Block>(ParseBlock());
                    break;
                case Token::kLet:
                    expr = make_shared<Let>(ParseLet());
                    break;
                case Token::kCase:
                    expr = make_shared<Case>(ParseCase());
                    break;
                case Token::kNew:
                    expr = make_shared<New>(ParseNew());
                    break;
                case Token::kIsvoid:
                    expr = make_shared<IsVoid>(ParseIsVoid());
                    break;
                case Token::kNegate:
                    expr = make_shared<Negate>(ParseNegate());
                    break;
                case Token::kNot:
                    expr = make_shared<Not>(ParseNot());
                    break;
                case Token::kOpenParen:
                    expr = ParseExpr();
                    ConsumeIfMatchWithException(Token::kCloseParen, "")
                    break;
                case Token::ID: {
                    if (MatchMultiple({Token::ID, Token::kAssignment})) {
                        expr = make_shared<Assign>(ParseAssign());
                    } else if (MatchMultiple({Token::ID, Token::kOpenParen})) {
                        expr = make_shared<Call>(ParseCall());;
                    } else {
                        expr = make_shared<ID>(ParseID());
                    }
                    break;
                }
                case Token::Integer:
                    expr = make_shared<Integer>(Integer());
                    break;
                case Token::String:
                    expr = make_shared<String>(String());
                    break;
                case Token::kTrue:
                    expr = make_shared<True>(True());
                    break;
                case Token::kFalse:
                    expr = make_shared<False>(False());
                    break;
                default:
                    throw runtime_error("invalid Expr type");
            }
        }
        else if (tokType >= Token::kBinaryST && tokType <= Token::kBinaryEND) {
            if (exprStack.empty()) {
                throw runtime_error("");
            }
            auto& right = exprStack.top();
            exprStack.pop();
            switch (tokType) {
                case Token::kAdd:
                    expr = make_shared<Add>(ParseAdd(right));
                    break;
                case Token::kMinus:
                    expr = make_shared<Minus>(ParseMinus(right));
                    break;
                case Token::kMultiply:
                    expr = make_shared<Multiply>(ParseMultiply(right));
                    break;
                case Token::kDivide:
                    expr = make_shared<Divide>(ParseDivide(right));
                    break;
                case Token::kLessThan:
                    expr = make_shared<LessThan>(ParseLessThan(right));
                    break;
                case Token::kLessThanOrEqual:
                    expr = make_shared<LessThanOrEqual>(ParseLessThanOrEqual(right));
                    break;
                case Token::kDot:
                    expr = make_shared<MethodCall>(ParseMethodCall(right));
                    break;
                case Token::kEqual:
                    expr = make_shared<Equal>(ParseEqual(right));
                    break;
            }
            exprStack.push(expr);
        }
        else {
            return exprStack.top();
        }
        exprStack.push(expr);
    }
}

If Parser::ParseIf() {
    If anIf;
    ConsumeIfMatchWithException(Token::kIf, "")
    anIf.ifExpr = ParseExpr();
    ConsumeIfMatchWithException(Token::kThen, "")
    anIf.thenExpr = ParseExpr();
    ConsumeIfMatchWithException(Token::kElse, "")
    anIf.elseExpr = ParseExpr();
    ConsumeIfMatchWithException(Token::kFi, "")
    return anIf;
}

Block Parser::ParseBlock() {
    Block blk;
    ConsumeIfMatchWithException(Token::kOpenBrace, "")
    while (true) {
        blk.exprs.emplace_back(ParseExpr());
        ConsumeIfMatchWithException(Token::kSemiColon, "")
        if (ConsumeIfMatch(Token::kCloseBrace)) {
            break;
        }
    }
    return blk;
}

While Parser::ParseWhile() {
    While aWhile;
    ConsumeIfMatchWithException(Token::kWhile, "")
    aWhile.whileExpr = ParseExpr();
    ConsumeIfMatchWithException(Token::kLoop, "")
    aWhile.loopExpr = ParseExpr();
    ConsumeIfMatchWithException(Token::kPool, "")
    return aWhile;
}

Let Parser::ParseLet() {
    Let let;
    ConsumeIfMatchWithException(Token::kLet, "")
    auto parseFormal = [&](){
        Let::Formal formalDef;
        AssignIfMatchWithException(Token::ID, formalDef.name = ConsumeReturn().str, "")
        ConsumeIfMatchWithException(Token::kColon, "")
        AssignIfMatchWithException(Token::TypeID, formalDef.type = ConsumeReturn().str, "")
        if (ConsumeIfMatch(Token::kAssignment)) {
            formalDef.expr = ParseExpr();
        }
        return formalDef;
    };
    let.formals.emplace_back(make_shared<Let::Formal>(parseFormal()));
    while (ConsumeIfMatch(Token::kComma)) {
        let.formals.emplace_back(make_shared<Let::Formal>(parseFormal()));
    }
    ConsumeIfMatchWithException(Token::kIn, "")
    let.expr = ParseExpr();
    return let;
}

Case Parser::ParseCase() {
    Case aCase;
    ConsumeIfMatchWithException(Token::kCase, "")
    aCase.expr = ParseExpr();
    ConsumeIfMatchWithException(Token::kOf, "")
    auto parseBranch = [&](){
        Case::branch branchDef;
        AssignIfMatchWithException(Token::ID, branchDef.id = ConsumeReturn().str, "")
        ConsumeIfMatchWithException(Token::kColon, "")
        AssignIfMatchWithException(Token::TypeID, branchDef.type = ConsumeReturn().str, "")
        ConsumeIfMatchWithException(Token::kEval, "")
        branchDef.expr = ParseExpr();
        ConsumeIfMatchWithException(Token::kSemiColon, "")
        return branchDef;
    };
    aCase.branches.emplace_back(make_shared<Case::branch>(parseBranch()));
    while (!ConsumeIfMatch(Token::kEsac)) {
        aCase.branches.emplace_back(make_shared<Case::branch>(parseBranch()));
    }
    return aCase;
}

ID Parser::ParseID() {
    ID id;
    AssignIfMatchWithException(Token::ID, id.name = ConsumeReturn().str, "")
    return id;
}

Assign Parser::ParseAssign() {
    Assign assign;
    assign.id = make_shared<ID>(ParseID());
    ConsumeIfMatchWithException(Token::kAssignment, "")
    assign.expr = ParseExpr();
    return assign;
}

Call Parser::ParseCall() {
    Call call;
    call.id = make_shared<ID>(ParseID());
    call.args = MatchSequence<shared_ptr<Expr>>(
        Token::kOpenParen,
        Token::kComma,
        Token::kCloseParen,
        [&](){ return ParseExpr(); });
    return call;
}

New Parser::ParseNew() {
    New aNew;
    ConsumeIfMatchWithException(Token::kNew, "")
    AssignIfMatchWithException(Token::TypeID, aNew.type = ConsumeReturn().str, "")
    return aNew;
}

IsVoid Parser::ParseIsVoid() {
    ConsumeIfMatchWithException(Token::kIsvoid, "")
    return IsVoid(ParseExpr());
}

Negate Parser::ParseNegate() {
    ConsumeIfMatchWithException(Token::kNegate, "")
    return Negate(ParseExpr());
}

Not Parser::ParseNot() {
    ConsumeIfMatchWithException(Token::kNot, "")
    return Not(ParseExpr());
}

Add Parser::ParseAdd(shared_ptr<repr::Expr> left) {
    ConsumeIfMatchWithException(Token::kAdd, "")
    return Add(move(left), ParseExpr());
}

Minus Parser::ParseMinus(shared_ptr<repr::Expr> left) {
    ConsumeIfMatchWithException(Token::kMinus, "")
    return Minus(move(left), ParseExpr());
}

Multiply Parser::ParseMultiply(shared_ptr<repr::Expr> left) {
    ConsumeIfMatchWithException(Token::kMultiply, "")
    return Multiply(move(left), ParseExpr());
}
Divide Parser::ParseDivide(shared_ptr<repr::Expr> left) {
    ConsumeIfMatchWithException(Token::kDivide, "")
    return Divide(move(left), ParseExpr());
}

LessThan Parser::ParseLessThan(shared_ptr<repr::Expr> left) {
    ConsumeIfMatchWithException(Token::kLessThan, "")
    return LessThan(move(left), ParseExpr());
}
LessThanOrEqual Parser::ParseLessThanOrEqual(shared_ptr<repr::Expr> left) {
    ConsumeIfMatchWithException(Token::kLessThanOrEqual, "")
    return LessThanOrEqual(move(left), ParseExpr());
}

Equal Parser::ParseEqual(shared_ptr<repr::Expr> left) {
    ConsumeIfMatchWithException(Token::kEqual, "")
    return Equal(move(left), ParseExpr());
}

MethodCall Parser::ParseMethodCall(shared_ptr<repr::Expr> left) {
    ConsumeIfMatchWithException(Token::kDot, "")
    return MethodCall(move(left), ParseExpr());
}