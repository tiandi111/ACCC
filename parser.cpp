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

bool ParsingResultChecker::Visit(repr::Program &prog) {
    for (auto& cls : prog.classes) if (!cls || !Visit(*cls)) return false;
    return true;
}

bool ParsingResultChecker::Visit(repr::Class &cls) {
    if (cls.name.Empty()) return false;
    for (auto& feat : cls.fields) if (!feat || !Visit(*feat)) return false;
    for (auto& feat : cls.funcs) if (!feat || !Visit(*feat)) return false;
    return true;
}

bool ParsingResultChecker::Visit(repr::FuncFeature &feat) {
    for (auto& arg : feat.args) if (!arg || !Visit(*arg)) return false;
    return !feat.name.Empty() && !feat.type.Empty() && feat.expr && Visit(*feat.expr);
}

bool ParsingResultChecker::Visit(repr::FieldFeature &feat) {
    return !feat.name.Empty() && !feat.type.Empty() && (!feat.expr || (feat.expr && Visit(*feat.expr)));
}

bool ParsingResultChecker::Visit(repr::Formal &form) {
    return !form.name.Empty() && !form.type.Empty();
}

bool ParsingResultChecker::Visit(repr::Let::Formal &form) {
    return !form.name.Empty() && !form.type.Empty() && (!form.expr || (form.expr && Visit(*form.expr)));

}

bool ParsingResultChecker::Visit(repr::Expr& expr) {
    return ExprVisitor<bool>::Visit(expr);
}

bool ParsingResultChecker::Visit(shared_ptr<repr::Expr>& expr) {
    return expr && ExprVisitor<bool>::Visit(*expr);
}

bool ParsingResultChecker::Visit_(repr::LinkBuiltin& expr) {
    return true;
}

bool ParsingResultChecker::Visit_(repr::Assign& expr) {
    return expr.id && Visit_(*expr.id) && Visit(expr.expr);
}

bool ParsingResultChecker::Visit_(repr::Add& expr) {
    return Visit(expr.left) && Visit(expr.right);
}

bool ParsingResultChecker::Visit_(repr::Block& expr) {
    for (auto& e : expr.exprs) if (!Visit(e)) return false;
    return true;
}

bool ParsingResultChecker::VisitBinary(repr::Binary& expr) {
    return Visit(expr.left) && Visit(expr.right);
}

bool ParsingResultChecker::Visit_(repr::Case& expr) {
    if (expr.branches.empty()) return false;
    for (auto& branch : expr.branches) if (!branch || !Visit(*branch)) return false;
    return Visit(expr.expr);
}

bool ParsingResultChecker::Visit(repr::Case::Branch& branch) {
    return !branch.id.Empty() && !branch.type.Empty() && Visit(branch.expr);
}

bool ParsingResultChecker::Visit_(repr::Call& expr) {
    for (auto& arg : expr.args) if (!arg && !Visit(*arg)) return false;
    return expr.id && Visit_(*expr.id);
}

bool ParsingResultChecker::Visit_(repr::Divide& expr) { return VisitBinary(expr); }

bool ParsingResultChecker::Visit_(repr::Equal& expr) { return VisitBinary(expr); }
bool ParsingResultChecker::Visit_(repr::False& expr) { return true; }
bool ParsingResultChecker::Visit_(repr::ID& expr) { return !expr.name.Empty(); }
bool ParsingResultChecker::Visit_(repr::IsVoid& expr) { return Visit(expr.expr); }
bool ParsingResultChecker::Visit_(repr::Integer& expr) { return true; }

bool ParsingResultChecker::Visit_(repr::If& expr) {
    return Visit(expr.ifExpr) && Visit(expr.thenExpr) && Visit(expr.elseExpr);
}

bool ParsingResultChecker::Visit_(repr::LessThanOrEqual& expr) { return VisitBinary(expr); }
bool ParsingResultChecker::Visit_(repr::LessThan& expr) { return VisitBinary(expr); }

bool ParsingResultChecker::Visit_(repr::Let& expr) {
    for (auto& form : expr.formals)
        if (!form || form->name.Empty() || form->type.Empty() || (form->expr && !Visit(form->expr)))
            return false;
    return Visit(expr.expr);
}

bool ParsingResultChecker::Visit_(repr::MethodCall& expr) { return VisitBinary(expr); }
bool ParsingResultChecker::Visit_(repr::Multiply& expr) { return VisitBinary(expr); }
bool ParsingResultChecker::Visit_(repr::Minus& expr) { return VisitBinary(expr); }
bool ParsingResultChecker::Visit_(repr::Negate& expr) { return Visit(expr.expr); }
bool ParsingResultChecker::Visit_(repr::New& expr) { return !expr.type.Empty(); }
bool ParsingResultChecker::Visit_(repr::Not& expr) { return Visit(expr.expr); }
bool ParsingResultChecker::Visit_(repr::String& expr) { return true; }
bool ParsingResultChecker::Visit_(repr::True& expr) { return true; }
bool ParsingResultChecker::Visit_(repr::While& expr) { return Visit(expr.whileExpr) && Visit(expr.loopExpr); }

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

diag::TextInfo Parser::GetTextInfo() {
    if (pos >= toks.size()) return toks.back().textInfo;
    return toks.at(pos).textInfo;
}

// note: the parsed ast must only contains valid nodes so that the semantic checking can be easier,
//  i.e., they only need to check valid nodes. for this reason, the checker need only checks expr,
//  for any grammer that contains expr, the grammer parser itseld should filter out invalid exprs.
Program Parser::ParseProgram() {
    Program prog(Peek().textInfo);

    while (!Empty()) {

        if (Match(Token::kClass)) {
            auto cls = ParseClass();
            if (checker.Visit(cls)) prog.classes.emplace_back(make_shared<Class>(cls));
        } else {
            diag.EmitError(GetTextInfo(), "expected 'class' in class declaration");
            SkipTo({Token::kClass});
        }

    }

    return prog;
}

Class Parser::ParseClass() {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kClass), "unexpected call to ParseClass");
    Class cls;

    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::TypeID), cls.name = StringAttr(ConsumeReturn()),
        "expected type identifier(start with capital letter)", cls);

    if (ConsumeIfMatch(Token::kInheirits))
        PARSER_STAT_IF_FALSE_EMIT_DIAG_SKIP(Match(Token::TypeID), cls.parent = StringAttr(ConsumeReturn()),
            "expected type identifier(start with capital letter) after 'inherits'")

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::kOpenBrace), "expected '{' in class declaration", cls);
    int closeBracePos = ReturnValidBraceMatchFor(Pos());
    if (closeBracePos < 0) {
        MoveTo(ScopeEnd()-1);
        diag.EmitError(GetTextInfo(), "expected '}' in class declaration");
        return cls;
    }
    Consume();

    PushScopeEnd(closeBracePos);
    while (!Empty()) {
        if (MatchMultiple({Token::ID, Token::kOpenParen})) {
            auto feat = ParseFuncFeature();
            if (checker.Visit(feat)) cls.funcs.emplace_back(make_shared<FuncFeature>(feat));
        } else if (Match(Token::ID)) {
            auto feat = ParseFieldFeature();
            if (checker.Visit(feat)) cls.fields.emplace_back(make_shared<FieldFeature>(feat));
        } else {
            diag.EmitError(GetTextInfo(), "expected identifier in class feature declaration");
            break;
        }
    }
    PopScopeEnd();

    MoveTo(closeBracePos+1);

    if (!ConsumeIfMatch(Token::kSemiColon))
        diag.EmitError(GetTextInfo(), "expected ';' after class declaration");
    return cls;
}

FuncFeature Parser::ParseFuncFeature() {
    FuncFeature feat;

    PARSER_IF_FALSE_EXCEPTION(MatchMultiple({Token::ID, Token::kOpenParen}), "unexpected call to ParseFuncFeature");
    feat.name = StringAttr(ConsumeReturn());

    int closeParenPos = ReturnValidParenMatchFor(Pos());
    if (closeParenPos < 0) {
        MoveTo(ScopeEnd()-1);
        diag.EmitError(GetTextInfo(), "expected ')' in class method definition");
        return feat;
    }
    Consume();

    PushScopeEnd(closeParenPos);
    while (!Empty()) {
        if (!feat.args.empty() && !ConsumeIfMatch(Token::kComma)) {
            diag.EmitError(GetTextInfo(), "expected ',' after formal parameter");
            break;
        }
        auto form = ParseFormal();
        if (checker.Visit(form)) feat.args.emplace_back(make_shared<Formal>(form));
        else break;
    }
    PopScopeEnd();
    MoveTo(closeParenPos+1);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kColon), "expected ':' in class method definition", feat);

    PARSER_STAT_IF_FALSE_EMIT_DIAG_SKIP(Match(Token::TypeID), feat.type = StringAttr(ConsumeReturn()),
        "expected type identifier in class method definition");

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::kOpenBrace), "expected '{' in class method definition", feat);

    int closeBracePos = ReturnValidBraceMatchFor(Pos());
    if (closeBracePos < 0) {
        MoveTo(ScopeEnd()-1);
        diag.EmitError(GetTextInfo(), "expected '}' in class method definition");
        return feat;
    }
    Consume();

    PushScopeEnd(closeBracePos);
    auto expr = ParseExpr();
    if (!Empty()) diag.EmitError(GetTextInfo(), "only one expression allowed in function declaration");
    PopScopeEnd();

    if (checker.Visit(expr)) feat.expr = expr;
    MoveTo(closeBracePos+1);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kSemiColon), "expected ';' in class method definition", feat);
    return feat;
}

FieldFeature Parser::ParseFieldFeature() {
    FieldFeature feat;

    PARSER_IF_FALSE_EXCEPTION(Match(Token::ID), "unexpected call to ParseFieldFeature");
    feat.name = StringAttr(ConsumeReturn());

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kColon), "expected ':' in class field declaration", feat);

    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::TypeID), feat.type = StringAttr(ConsumeReturn()),
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

    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::ID), form.name = StringAttr(ConsumeReturn()),
                                          "expected parameter identifier in formal parameter declaration", form);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kColon), "expected ':' in formal parameter declaration", form);

    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(
        Match(Token::TypeID),
        PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(
            Peek().val != "SELF_TYPE",
            form.type = StringAttr(ConsumeReturn()),
            "'SELF_TYPE' cannot be used in formal parameter declaration", form),
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
                        diag.EmitError(GetTextInfo(), "expected '}' in block expression");
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
                        diag.EmitError(GetTextInfo(), "expected ')'");
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
                            diag.EmitError(GetTextInfo(), "expected ')'");
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
    Block blk(GetTextInfo());

    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kOpenBrace), "unexpected call to ParseBlock");

    while (!Empty()) {
        auto expr = ParseExpr();
        if (checker.Visit(expr)) blk.exprs.emplace_back(expr);
        else break;

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
        formalDef.name = StringAttr(ConsumeReturn());

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kColon), "expected keyword ':'", formalDef);

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::TypeID), "expected type identifier", formalDef);
        formalDef.type = StringAttr(ConsumeReturn());

        if (ConsumeIfMatch(Token::kAssignment)) {
            auto expr = ParseExpr();
            PARSER_CHECK_ASSGIN_RETURN(formalDef.expr, expr, formalDef);
        }

        return formalDef;
    };

    do {
        auto form = parseFormal();
        if (checker.Visit(form)) let.formals.emplace_back(make_shared<Let::Formal>(form));
        else return let;

    } while (ConsumeIfMatch(Token::kComma));

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kIn), "expected 'in' in let expression", let);

    auto expr = ParseExpr();
    PARSER_CHECK_ASSGIN_RETURN(let.expr, expr, let);
    return let;
}

Case Parser::ParseCase() {
    Case aCase;

    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kCase), "expected keyword 'case'")
    auto expr = ParseExpr();
    PARSER_CHECK_ASSGIN_RETURN(aCase.expr, expr, aCase);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kOf), "expected 'of' in case expression", aCase);

    auto parseBranch = [&](){
        Case::Branch branch;

        PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::ID), branch.id = StringAttr(ConsumeReturn()),
            "expected identifier", branch);

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kColon), "expected ':'", branch);

        PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(
            Match(Token::TypeID),
            PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(
                Peek().val != "SELF_TYPE",
                branch.type = StringAttr(ConsumeReturn()),
                "'SELF_TYPE' cannot be used in case expression", branch),
            "expected type identifier", branch);

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kEval), "expected '=>'", branch);

        auto expr = ParseExpr();
        PARSER_CHECK_ASSGIN_RETURN(branch.expr, expr, branch);

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kSemiColon), "expected ';'", branch);
        return branch;
    };

    do {
        auto branch = parseBranch();
        if (checker.Visit(branch)) aCase.branches.emplace_back(make_shared<Case::Branch>(branch));
        else return aCase;
    } while (!ConsumeIfMatch(Token::kEsac));

    return aCase;
}

ID Parser::ParseID() {
    ID id;
    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::ID), id.name = StringAttr(ConsumeReturn()),
        "expected identifier", id);
    return id;
}

Assign Parser::ParseAssign() {
    Assign assign;
    auto id = ParseID();
    if (checker.Visit(id)) assign.id = make_shared<ID>(id);
    else return assign;
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kAssignment), "unexpected call to ParseAssign")
    auto expr = ParseExpr();
    PARSER_CHECK_ASSGIN_RETURN(assign.expr, expr, assign);
    return assign;
}

Call Parser::ParseCall() {
    Call call;
    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::ID), call.id = make_shared<ID>(ParseID()),
        "expected identifier in call", call);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::kOpenParen), "expected '(' in call expression", call);
    int end = ReturnValidParenMatchFor(Pos());
    PARSER_IF_FALSE_EMIT_DIAG_RETURN((end >= 0), "expected ')' in call expression", call);
    Consume();

    PushScopeEnd(end);
    while (!Empty()) {
        // note: if parse expr failed, should emplace nullptr so that checker can decide its invalidity
        if(!call.args.empty() && !ConsumeIfMatch(Token::kComma)) {
            diag.EmitError(GetTextInfo(), "expected ',' after expression");
            break;
        }
        auto expr = ParseExpr();
        if (checker.Visit(expr)) call.args.emplace_back(expr);
        else break;
    }
    PopScopeEnd();
    MoveTo(end+1);
    return call;
}

New Parser::ParseNew() {
    New aNew;
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kNew), "unexpected call to ParseNew");

    if (!ConsumeIfNotMatch(Token::TypeID)) aNew.type = StringAttr(ConsumeReturn());
    else diag.EmitError(GetTextInfo(), "expected type identifier in new expression");

    return aNew;
}

repr::Integer Parser::ParseInteger() {
    PARSER_IF_FALSE_EXCEPTION(Match(Token::Integer), "unexpected call to ParseInteger");
    return Integer(ConsumeReturn());
}

repr::String Parser::ParseString() {
    PARSER_IF_FALSE_EXCEPTION(Match(Token::String), "unexpected call to ParseString");
    return String(ConsumeReturn());
}

repr::True Parser::ParseTrue() {
    PARSER_IF_FALSE_EXCEPTION(Match(Token::kTrue), "unexpected call to ParseTrue");
    return True(ConsumeReturn().textInfo);
}

repr::False Parser::ParseFalse() {
    PARSER_IF_FALSE_EXCEPTION(Match(Token::kFalse), "unexpected call to ParseFalse");
    return False(ConsumeReturn().textInfo);
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