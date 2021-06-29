//
// Created by 田地 on 2021/5/30.
//

#include <stdexcept>
#include <vector>
#include <stack>
#include <unordered_set>

#include "parser.h"
#include "repr.h"
#include "tokenizer.h"
#include "token.h"

using namespace cool;
using namespace parser;
using namespace repr;
using namespace tok;

Parser::Parser(diag::Diagnosis& _diag, vector<Token> _toks)
: diag(_diag), toks(std::move(_toks)), pos(0) {
    scopeEnds.push(toks.size());
}


void Parser::Consume() {
    if (pos >= ScopeEnd()) throw runtime_error("call Consume out of scope");
    pos++;
}

Token Parser::ConsumeReturn() {
    if (pos >= ScopeEnd()) throw runtime_error("call Consume out of scope");
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

bool Parser::ConsumeIfNotMatch(Token::Type type) {
    if (!Match(type)) {
        Consume();
        return true;
    }
    return false;
}

bool Parser::Match(Token::Type type) {
    return !Empty() && toks[pos].type == type;
}

bool Parser::MatchMultiple(vector<Token::Type> types) {
    if ((ScopeEnd() - pos) < types.size()) return false;
    for (int i = pos; i < pos + types.size(); i++) {
        if (types.at(i-pos) != toks.at(i).type) return false;
    }
    return true;
}

bool Parser::SkipTo(unordered_set<Token::Type> types) {
    while (pos < ScopeEnd() && (types.find(toks.at(pos).type) == types.end())) pos++;
    return pos < ScopeEnd();
}

void Parser::PushScopeEnd(int scopeEnd) {
    scopeEnds.push(scopeEnd);
}

int Parser::PopScopeEnd() {
    if (scopeEnds.size() == 1) throw runtime_error("no scope end to pop");
    scopeEnds.pop();
}

int Parser::ReturnNextPos(Token::Type type) {
    for (int i = pos; i < ScopeEnd(); i++) {
        if (toks.at(i).type == type) return i;
    }
    return -1;
}

int Parser::ReturnValidParenMatchFor(int p) {
    if (toks.at(p).type != Token::kOpenParen) throw runtime_error("given token is not a open parenthsis");
    int open = 0;
    for (; p < ScopeEnd(); p++) {
        if (toks.at(p).type == Token::kOpenParen) open++;
        if (toks.at(p).type == Token::kCloseParen) open--;
        if (open == 0) return p;
    }
    return -1;
}

int Parser::ReturnValidBraceMatchFor(int p) {
    if (toks.at(p).type != Token::kOpenBrace) throw runtime_error("given token is not a open brace");
    int open = 0;
    for (; p < ScopeEnd(); p++) {
        if (toks.at(p).type == Token::kOpenBrace) open++;
        if (toks.at(p).type == Token::kCloseBrace) open--;
        if (open == 0) return p;
    }
    return -1;
}

int Parser::ScopeEnd() {
    return scopeEnds.top();
}

void Parser::MoveTo(int cur) {
    pos = cur;
}

Token& Parser::Peek() {
    if (pos >= ScopeEnd()) throw runtime_error("call Peek out of scope");
    return toks[pos];
}

bool Parser::Empty() {
    return pos >= ScopeEnd();
}

int Parser::Pos() {
    return pos;
}

int Parser::TextLine() {
    if (pos >= toks.size()) return toks.back().line;
    return toks.at(pos).line;
}

int Parser::TextPos() {
    if (pos >= toks.size()) return toks.back().pos;
    return toks.at(pos).pos;
}

string Parser::FileName() {
    if (pos >= toks.size()) return FileMapper::GetFileMapper().GetFileName(toks.back().fileno);
    return FileMapper::GetFileMapper().GetFileName(toks.at(pos).fileno);
}

Program Parser::ParseProgram() {
    Program prog;

    while (!Empty()) {

        if (Match(Token::kClass)) {
            prog.classes.emplace_back(make_shared<Class>(ParseClass()));
        } else {
            diag.EmitError(FileName(), TextLine(), TextPos(), "expected class declaration");
            SkipTo({Token::kClass});
        }

    }

    if (prog.classes.empty()) diag.EmitError(FileName(),TextLine(), TextPos(), "'Main' class not declared");

    return prog;
}

Class Parser::ParseClass() {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kClass), "unexpected call to ParseClass");
    Class cls;

    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::TypeID), cls.name = ConsumeReturn().str,
        "expected type identifier(start with capital letter)", cls);

    if (ConsumeIfMatch(Token::kInheirits)) {
        PARSER_STAT_IF_FALSE_EMIT_DIAG_SKIP(Match(Token::TypeID), cls.parent = ConsumeReturn().str,
                                            "expected type identifier(start with capital letter) after 'inherits'")
    }

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::kOpenBrace), "expected '{' in class declaration", cls);
    int closeBracePos = ReturnValidBraceMatchFor(Pos());
    if (closeBracePos < 0) {
        MoveTo(ScopeEnd()-1);
        diag.EmitError(FileName(), TextLine(), TextPos(), "expected '}' in class declaration");
        return cls;
    }
    Consume();

    PushScopeEnd(closeBracePos);
    while (!Empty()) {
        if (MatchMultiple({Token::ID, Token::kOpenParen})) {
            cls.funcs.emplace_back(make_shared<FuncFeature>(ParseFuncFeature()));
            if (!checker.Visit(*cls.funcs.back())) break;
        } else if (Match(Token::ID)) {
            cls.fields.emplace_back(make_shared<FieldFeature>(ParseFieldFeature()));
            if (!checker.Visit(*cls.fields.back())) break;
        } else {
            diag.EmitError(FileName(),TextLine(), TextPos(), "expected identifier in class feature declaration");
            break;
        }
    }
    PopScopeEnd();

    MoveTo(closeBracePos+1);

    if (!ConsumeIfMatch(Token::kSemiColon))
        diag.EmitError(FileName(),TextLine(), TextPos(), "expected ';' after class declaration");
    return cls;
}

FuncFeature Parser::ParseFuncFeature() {
    FuncFeature feat;

    PARSER_IF_FALSE_EXCEPTION(MatchMultiple({Token::ID, Token::kOpenParen}), "unexpected call to ParseFuncFeature");
    feat.name = ConsumeReturn().str;

    int closeParenPos = ReturnValidParenMatchFor(Pos());
    if (closeParenPos < 0) {
        MoveTo(ScopeEnd()-1);
        diag.EmitError(FileName(),TextLine(), TextPos(), "expected ')' in class method definition");
        return feat;
    }
    Consume();

    PushScopeEnd(closeParenPos);
    vector<Formal> formals;
    while (!Empty()) {
        if (!formals.empty() && !ConsumeIfMatch(Token::kComma)) {
            diag.EmitError(FileName(),TextLine(), TextPos(), "expected ',' after formal parameter");
            break;
        }
        formals.emplace_back(ParseFormal());
        if (!checker.Visit(formals.back())) break;
    }
    PopScopeEnd();
    MoveTo(closeParenPos+1);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kColon), "expected ':' in class method definition", feat);

    PARSER_STAT_IF_FALSE_EMIT_DIAG_SKIP(Match(Token::TypeID), feat.name = ConsumeReturn().str,
        "expected type identifier in class method definition");

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kOpenBrace), "expected '{' in class method definition", feat);

    auto expr = ParseExpr();
    PARSER_CHECK_ASSGIN_RETURN(feat.expr, expr, feat);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kCloseBrace), "expected '}' in class method definition", feat);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kSemiColon), "expected ';' in class method definition", feat);
    return feat;
}

FieldFeature Parser::ParseFieldFeature() {
    FieldFeature feat;

    PARSER_IF_FALSE_EXCEPTION(Match(Token::ID), "unexpected call to ParseFieldFeature");
    feat.name = ConsumeReturn().str;

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kColon), "expected ':' in class field declaration", feat);

    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::TypeID), feat.type = ConsumeReturn().str,
                                          "expected type identifier in class field declaration", feat);


    if (ConsumeIfMatch(Token::kAssignment)) {
        auto expr = ParseExpr();
        PARSER_CHECK_ASSGIN_RETURN(feat.expr, expr, feat)
    }

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kSemiColon), "expected ';' after class field declaration", feat);
    return feat;
}

Formal Parser::ParseFormal() {
    Formal form;

    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::ID), form.name = ConsumeReturn().str,
                                          "expected parameter identifier in formal parameter declaration", form);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kColon), "expected ':' in formal parameter declaration", form);

    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::TypeID), form.type = ConsumeReturn().str,
                                          "expected type identifier in formal parameter declaration", form);
    return form;
}

shared_ptr<Expr> Parser::ParseExpr() {
    stack<shared_ptr<Expr>> exprStack;

    while (!Empty()) {
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
                case Token::kOpenBrace: {
                    int closeBracePos = ReturnValidBraceMatchFor(Pos());
                    if (closeBracePos < 0) {
                        MoveTo(ScopeEnd() - 1);
                        diag.EmitError(FileName(),TextLine(), TextPos(), "expected '}' in block expression");
                        expr = nullptr;
                        break;
                    }
                    PushScopeEnd(closeBracePos + 1);
                    expr = make_shared<Block>(ParseBlock());
                    PopScopeEnd();
                    MoveTo(closeBracePos + 1);
                    break;
                }
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
                case Token::kOpenParen: {
                    int closeParenPos = ReturnValidParenMatchFor(Pos());
                    if (closeParenPos < 0) {
                        MoveTo(ScopeEnd()-1);
                        diag.EmitError(FileName(),TextLine(), TextPos(), "expected ')'");
                        expr = nullptr;
                        break;
                    }
                    Consume();
                    PushScopeEnd(closeParenPos);
                    expr = ParseExpr();
                    PopScopeEnd();
                    MoveTo(closeParenPos + 1);
                    break;
                }
                case Token::ID: {
                    if (MatchMultiple({Token::ID, Token::kAssignment})) {
                        expr = make_shared<Assign>(ParseAssign());
                    } else if (MatchMultiple({Token::ID, Token::kOpenParen})) {
                        int closeParenPos = ReturnValidParenMatchFor(Pos()+1);
                        if (closeParenPos < 0) {
                            MoveTo(ScopeEnd() - 1);
                            diag.EmitError(FileName(),TextLine(), TextPos(), "expected ')'");
                            expr = nullptr;
                            break;
                        }
                        PushScopeEnd(closeParenPos + 1);
                        expr = make_shared<Call>(ParseCall());
                        PopScopeEnd();
                        MoveTo(closeParenPos + 1);
                    } else {
                        expr = make_shared<ID>(ParseID());
                    }
                    break;
                }
                case Token::Integer:
                    expr = make_shared<Integer>(ParseInteger());
                    break;
                case Token::String:
                    expr = make_shared<String>(ParseString());
                    break;
                case Token::kTrue:
                    expr = make_shared<True>(ParseTrue());
                    break;
                case Token::kFalse:
                    expr = make_shared<False>(ParseFalse());
                    break;
                default:
                    throw runtime_error("invalid Expr type");
            }
        }
        else if (tokType >= Token::kBinaryST && tokType <= Token::kBinaryEND) {
            if (exprStack.empty()) throw runtime_error("expr stack is empty");
            auto right = exprStack.top();
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
        }
        else {
            if (!checker.Visit(*exprStack.top())) return nullptr;
            return exprStack.top();
        }
        exprStack.push(expr);
    }

    if (exprStack.empty() || !checker.Visit(*exprStack.top())) return nullptr;
    return exprStack.top();
}

If Parser::ParseIf() {
    If anIf;

    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kIf), "unexpected call to ParseIf");
    auto expr = ParseExpr();
    PARSER_CHECK_ASSGIN_RETURN(anIf.ifExpr, expr, anIf);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kThen), "expected keyword 'then'", anIf);
    expr = ParseExpr();
    PARSER_CHECK_ASSGIN_RETURN(anIf.thenExpr, expr, anIf);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kElse), "expected keyword 'else'", anIf);
    expr = ParseExpr();
    PARSER_CHECK_ASSGIN_RETURN(anIf.elseExpr, expr, anIf);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kFi), "expected keyword 'fi'", anIf);
    return anIf;
}

Block Parser::ParseBlock() {
    Block blk;

    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kOpenBrace), "unexpected call to ParseBlock");

    while (!Empty()) {
        blk.exprs.emplace_back(ParseExpr());
        if (!checker.Visit(blk.exprs.back())) return blk;

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kSemiColon), "expected ';' after expression", blk);

        if (Match(Token::kCloseBrace)) break;
    }

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kCloseBrace), "expected '}' in block expression", blk);
    return blk;
}

While Parser::ParseWhile() {
    While aWhile;

    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kWhile), "unexpected call to ParseWhile")
    auto expr = ParseExpr();
    PARSER_CHECK_ASSGIN_RETURN(aWhile.whileExpr, expr, aWhile);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kLoop), "expected keyword 'loop'", aWhile);
    expr = ParseExpr();
    PARSER_CHECK_ASSGIN_RETURN(aWhile.loopExpr, expr, aWhile);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kPool), "expected keyword 'pool'", aWhile);
    return aWhile;
}

Let Parser::ParseLet() {
    Let let;

    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kLet), "unexpected call to ParseLet")

    auto parseFormal = [&](){
        Let::Formal formalDef;

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::ID), "expected identifier", formalDef);
        formalDef.name = ConsumeReturn().str;

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kColon), "expected keyword ':'", formalDef);

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::TypeID), "expected type identifier", formalDef);
        formalDef.type = ConsumeReturn().str;

        if (ConsumeIfMatch(Token::kAssignment)) {
            auto expr = ParseExpr();
            PARSER_CHECK_ASSGIN_RETURN(formalDef.expr, expr, formalDef);
        }

        return formalDef;
    };

    do {
        auto form = parseFormal();
        if (!checker.Visit(form)) return let;
        let.formals.emplace_back(make_shared<Let::Formal>(form));
    } while (ConsumeIfMatch(Token::kComma));

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kIn), "expected 'in' in let expression", let);

    let.expr = ParseExpr();
    return let;
}

Case Parser::ParseCase() {
    Case aCase;

    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kCase), "expected keyword 'case'")
    auto expr = ParseExpr();
    PARSER_CHECK_ASSGIN_RETURN(aCase.expr, expr, aCase);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kOf), "expected 'of' in case expression", aCase);

    auto parseBranch = [&](){
        Case::Branch branchDef;

        PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::ID), branchDef.id = ConsumeReturn().str,
            "expected identifier", branchDef);

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kColon), "expected ':'", branchDef);

        PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::TypeID), branchDef.type = ConsumeReturn().str,
                                            "expected type identifier", branchDef);

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kEval), "expected '=>'", branchDef);

        auto expr = ParseExpr();
        PARSER_CHECK_ASSGIN_RETURN(branchDef.expr, expr, branchDef);

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kSemiColon), "expected ';'", branchDef);
        return branchDef;
    };

    do {
        auto branch = parseBranch();
        if (!checker.Visit(branch)) return aCase;
        aCase.branches.emplace_back(make_shared<Case::Branch>(branch));
    } while (!ConsumeIfMatch(Token::kEsac));

    return aCase;
}

ID Parser::ParseID() {
    ID id;
    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::ID), id.name = ConsumeReturn().str, "expected identifier", id);
    return id;
}

Assign Parser::ParseAssign() {
    Assign assign;
    assign.id = make_shared<ID>(ParseID());
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kAssignment), "unexpected call to ParseAssign")
    assign.expr = ParseExpr();
    return assign;
}

Call Parser::ParseCall() {
    Call call;
    if (Match(Token::ID)) call.id = make_shared<ID>(ParseID());
    else {
        call.id = make_shared<ID>(ID());
        diag.EmitError(FileName(),TextLine(), TextPos(), "expected identifier in call");
    }

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::kOpenParen), "expected '(' in call expression", call);
    int end = ReturnValidParenMatchFor(Pos());
    PARSER_IF_FALSE_EMIT_DIAG_RETURN((end >= 0), "expected ')' in call expression", call);
    Consume();

    PushScopeEnd(end);
    while (!Empty()) {
        // todo: if parse expr failed, should emplace nullptr so that checker can decide its invalidity
        if(!call.args.empty() && !ConsumeIfMatch(Token::kComma)) {
            diag.EmitError(FileName(),TextLine(), TextPos(), "expected ',' after expression");
            call.args.emplace_back(nullptr);
            break;
        }
        call.args.emplace_back(ParseExpr());
        if (!checker.Visit(call.args.back())) break;
    }
    PopScopeEnd();
    MoveTo(end+1);
    return call;
}

New Parser::ParseNew() {
    New aNew;
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kNew), "unexpected call to ParseNew");

    if (!ConsumeIfNotMatch(Token::TypeID)) aNew.type = ConsumeReturn().str;
    else diag.EmitError(FileName(),TextLine(), TextPos(), "expected type identifier in new expression");

    return aNew;
}

repr::Integer Parser::ParseInteger() {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::Integer), "unexpected call to ParseInteger");
    return Integer();
}

repr::String Parser::ParseString() {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::String), "unexpected call to ParseString");
    return String();
}

repr::True Parser::ParseTrue() {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kTrue), "unexpected call to ParseTrue");
    return True();
}

repr::False Parser::ParseFalse() {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::Integer), "unexpected call to ParseFalse");
    return False();
}

IsVoid Parser::ParseIsVoid() {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kIsvoid), "unexpected call to ParseIsVoid");
    return IsVoid(ParseExpr());
}

Negate Parser::ParseNegate() {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kNegate), "unexpected call to ParseNegate");
    return Negate(ParseExpr());
}

Not Parser::ParseNot() {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kNot), "unexpected call to ParseNot");
    return Not(ParseExpr());
}

Add Parser::ParseAdd(shared_ptr<repr::Expr> left) {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kAdd), "unexpected call to ParseAdd");
    return Add(move(left), ParseExpr());
}

Minus Parser::ParseMinus(shared_ptr<repr::Expr> left) {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kMinus), "unexpected call to ParseMinus");
    return Minus(move(left), ParseExpr());
}

Multiply Parser::ParseMultiply(shared_ptr<repr::Expr> left) {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kMultiply), "unexpected call to ParseMultiply");
    return Multiply(move(left), ParseExpr());
}
Divide Parser::ParseDivide(shared_ptr<repr::Expr> left) {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kDivide), "unexpected call to ParseDivide");
    return Divide(move(left), ParseExpr());
}

LessThan Parser::ParseLessThan(shared_ptr<repr::Expr> left) {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kLessThan), "unexpected call to ParseLessThan");
    return LessThan(move(left), ParseExpr());
}
LessThanOrEqual Parser::ParseLessThanOrEqual(shared_ptr<repr::Expr> left) {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kLessThanOrEqual), "unexpected call to ParseLessThanOrEqual");
    return LessThanOrEqual(move(left), ParseExpr());
}

Equal Parser::ParseEqual(shared_ptr<repr::Expr> left) {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kEqual), "unexpected call to ParseEqual");
    return Equal(move(left), ParseExpr());
}

MethodCall Parser::ParseMethodCall(shared_ptr<repr::Expr> left) {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kDot), "unexpected call to ParseMethodCall");
    return MethodCall(move(left), make_shared<Call>(ParseCall()));
}