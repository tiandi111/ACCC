//
// Created by 田地 on 2021/5/30.
//

#include <stdexcept>
#include <vector>
#include <stack>
#include <unordered_set>

#include "parser.h"
#include "repr.h"
#include "token.h"
#include "constant.h"

using namespace cool;
using namespace parser;
using namespace repr;
using namespace tok;
using namespace constant;

bool ParsingResultChecker::Visit(repr::Program &prog) {
    for (auto& cls : prog.GetClasses()) if (!cls || !Visit(*cls)) return false;
    return true;
}

bool ParsingResultChecker::Visit(repr::Class &cls) {
    if (cls.GetName().Empty()) return false;
    for (auto& feat : cls.GetFieldFeatures()) if (!feat || !Visit(*feat)) return false;
    for (auto& feat : cls.GetFuncFeatures()) if (!feat || !Visit(*feat)) return false;
    return true;
}

bool ParsingResultChecker::Visit(repr::FuncFeature &feat) {
    for (auto& arg : feat.GetArgs())
        if (!arg || !Visit(*arg))
            return false;
    return !feat.GetName().Empty() && !feat.GetType().Empty() && feat.GetExpr() && Visit(feat.GetExpr());
}

bool ParsingResultChecker::Visit(repr::FieldFeature &feat) {
    return !feat.GetName().Empty() &&
    !feat.GetType().Empty() &&
    (!feat.GetExpr() || (feat.GetExpr() && Visit(feat.GetExpr())));
}

bool ParsingResultChecker::Visit(repr::Formal &form) {
    return !form.GetName().Empty() && !form.GetType().Empty();
}

bool ParsingResultChecker::Visit(repr::Expr* expr) {
    return expr && ExprVisitor<bool>::Visit(*expr);
}

bool ParsingResultChecker::Visit_(repr::LinkBuiltin& expr) {
    return true;
}

bool ParsingResultChecker::Visit_(repr::Assign& expr) {
    return expr.GetId() && Visit_(*expr.GetId()) && Visit(expr.GetExpr());
}

bool ParsingResultChecker::Visit_(repr::Add& expr) {
    return Visit(expr.GetLeft()) && Visit(expr.GetRight());
}

bool ParsingResultChecker::Visit_(repr::Block& expr) {
    for (auto& e : expr.GetExprs()) if (!Visit(e)) return false;
    return true;
}

bool ParsingResultChecker::VisitBinary(repr::Binary& expr) {
    return Visit(expr.GetLeft()) && Visit(expr.GetRight());
}

bool ParsingResultChecker::Visit_(repr::Case& expr) {
    if (expr.GetBranches().empty()) return false;
    for (auto& branch : expr.GetBranches())
        if (!branch || !Visit(*branch))
            return false;
    return Visit(expr.GetExpr());
}

bool ParsingResultChecker::Visit(repr::Case::Branch& branch) {
    return !branch.GetId().Empty() && !branch.GetType().Empty() && Visit(branch.GetExpr());
}

bool ParsingResultChecker::Visit_(repr::Call& expr) {
    for (auto& arg : expr.GetArgs())
        if (!arg && !Visit(arg)) return false;
    return expr.GetId() && Visit_(*expr.GetId());
}

bool ParsingResultChecker::Visit_(repr::Divide& expr) { return VisitBinary(expr); }

bool ParsingResultChecker::Visit_(repr::Equal& expr) { return VisitBinary(expr); }
bool ParsingResultChecker::Visit_(repr::False& expr) { return true; }
bool ParsingResultChecker::Visit_(repr::ID& expr) { return !expr.GetName().Empty(); }
bool ParsingResultChecker::Visit_(repr::IsVoid& expr) { return Visit(expr.GetExpr()); }
bool ParsingResultChecker::Visit_(repr::Integer& expr) { return true; }

bool ParsingResultChecker::Visit_(repr::If& expr) {
    return Visit(expr.GetIfExpr()) && Visit(expr.GetThenExpr()) && Visit(expr.GetElseExpr());
}

bool ParsingResultChecker::Visit_(repr::LessThanOrEqual& expr) { return VisitBinary(expr); }
bool ParsingResultChecker::Visit_(repr::LessThan& expr) { return VisitBinary(expr); }

bool ParsingResultChecker::Visit_(repr::Let& expr) {
    for (auto& decl : expr.GetDecls())
       if (!Visit(*decl))
           return false;
    return Visit(expr.GetExpr());
}

bool ParsingResultChecker::Visit(repr::Let::Decl& expr) {
    return !expr.GetName().Empty() && !expr.GetType().Empty() && (!expr.GetExpr() || Visit(expr.GetExpr()));
}

bool ParsingResultChecker::Visit_(repr::MethodCall& expr) { return VisitBinary(expr); }
bool ParsingResultChecker::Visit_(repr::Multiply& expr) { return VisitBinary(expr); }
bool ParsingResultChecker::Visit_(repr::Minus& expr) { return VisitBinary(expr); }
bool ParsingResultChecker::Visit_(repr::Negate& expr) { return Visit(expr.GetExpr()); }
bool ParsingResultChecker::Visit_(repr::New& expr) { return !expr.GetType().Empty(); }
bool ParsingResultChecker::Visit_(repr::Not& expr) { return Visit(expr.GetExpr()); }
bool ParsingResultChecker::Visit_(repr::String& expr) { return true; }
bool ParsingResultChecker::Visit_(repr::True& expr) { return true; }
bool ParsingResultChecker::Visit_(repr::While& expr) { return Visit(expr.GetWhileExpr()) && Visit(expr.GetLoopExpr()); }

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
Program* Parser::ParseProgram() {
    auto prog = new Program(Peek().textInfo);

    while (!Empty()) {

        if (Match(Token::kClass)) {
            auto cls = ParseClass();
            if (checker.Visit(*cls) && !prog->AddClass(cls)) {
                diag.EmitError(cls->GetTextInfo(), "class '" + cls->GetName().Value() +
                "' redefined, previous defined at: " +
                prog->GetClassPtr(cls->GetName().Value())->GetTextInfo().String());
            }
        } else {
            diag.EmitError(GetTextInfo(), "expected 'class' in class declaration");
            SkipTo({Token::kClass});
        }

    }

    return prog;
}

Class* Parser::ParseClass() {
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kClass), "unexpected call to ParseClass");

    auto cls = new Class();

    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::TypeID), cls->SetName(StringAttr(ConsumeReturn())),
        "expected type identifier(start with capital letter)", nullptr)

    if (ConsumeIfMatch(Token::kInheirits))
        PARSER_STAT_IF_FALSE_EMIT_DIAG_SKIP(Match(Token::TypeID), cls->SetParent(StringAttr(ConsumeReturn())),
            "expected type identifier(start with capital letter) after 'inherits'")

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::kOpenBrace), "expected '{' in class declaration", nullptr)
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
            if (checker.Visit(*feat) && !cls->AddFuncFeature(feat)) {
                diag.EmitError(feat->GetTextInfo(),"method '" + feat->GetName().Value() +
                "' redefined, previous declared at: " +
                cls->GetFuncFeaturePtr(feat->GetName().Value())->GetTextInfo().String());
            }
        } else if (Match(Token::ID)) {
            auto feat = ParseFieldFeature();
            if (checker.Visit(*feat) && !cls->AddFieldFeature(feat)) {
                    diag.EmitError(feat->GetTextInfo(),"attribute '" + feat->GetName().Value() +
                    "' redefined, previous declared at: " +
                    cls->GetFieldFeaturePtr(feat->GetName().Value())->GetTextInfo().String());
            }
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

FuncFeature* Parser::ParseFuncFeature() {
    auto feat = new FuncFeature();

    PARSER_IF_FALSE_EXCEPTION(MatchMultiple({Token::ID, Token::kOpenParen}),
        "unexpected call to ParseFuncFeature");
    feat->SetName(StringAttr(ConsumeReturn()));

    int closeParenPos = ReturnValidParenMatchFor(Pos());
    if (closeParenPos < 0) {
        MoveTo(ScopeEnd()-1);
        diag.EmitError(GetTextInfo(), "expected ')' in class method definition");
        return feat;
    }
    Consume();

    PushScopeEnd(closeParenPos);
    vector<Formal*> args;
    for (int i = 0; !Empty(); i++) {
        if (i > 0 && !ConsumeIfMatch(Token::kComma)) {
            diag.EmitError(GetTextInfo(), "expected ',' after formal parameter");
            break;
        }
        auto arg = ParseFormal();
        if (checker.Visit(*arg))
            args.emplace_back(arg);
        else break;
    }
    feat->SetArgs(args);
    PopScopeEnd();
    MoveTo(closeParenPos+1);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kColon),
        "expected ':' in class method definition", feat);

    PARSER_STAT_IF_FALSE_EMIT_DIAG_SKIP(Match(Token::TypeID), feat->SetType(StringAttr(ConsumeReturn())),
        "expected type identifier in class method definition");

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::kOpenBrace),
        "expected '{' in class method definition", feat);

    int closeBracePos = ReturnValidBraceMatchFor(Pos());
    if (closeBracePos < 0) {
        MoveTo(ScopeEnd()-1);
        diag.EmitError(GetTextInfo(), "expected '}' in class method definition");
        return feat;
    }
    Consume();

    PushScopeEnd(closeBracePos);
    auto expr = ParseExpr();
    if (!Empty())
        diag.EmitError(GetTextInfo(), "only one expression allowed in function declaration");
    PopScopeEnd();

    if (checker.Visit(expr))
        feat->SetExpr(expr);
    MoveTo(closeBracePos+1);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kSemiColon),
        "expected ';' in class method definition", feat);
    return feat;
}

FieldFeature* Parser::ParseFieldFeature() {
    auto feat = new FieldFeature();

    PARSER_IF_FALSE_EXCEPTION(Match(Token::ID), "unexpected call to ParseFieldFeature");
    feat->SetName(StringAttr(ConsumeReturn()));

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kColon),
        "expected ':' in class field declaration", feat);

    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::TypeID),
        feat->SetType(StringAttr(ConsumeReturn())),
        "expected type identifier in class field declaration", feat);


    if (ConsumeIfMatch(Token::kAssignment)) {
        auto expr = ParseExpr();
        PARSER_CHECK_STAT_IF_FALSE_RETURN(expr, feat->SetExpr(expr), feat)
    }

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kSemiColon),
        "expected ';' after class field declaration", feat);
    return feat;
}

Formal* Parser::ParseFormal() {
    auto formal = new Formal();

    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::ID),
        formal->SetName(StringAttr(ConsumeReturn())),
        "expected parameter identifier in formal parameter declaration", formal);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kColon),
        "expected ':' in formal parameter declaration", formal);

    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(
        Match(Token::TypeID),
        PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(
            Peek().val != TYPE_SELF_TYPE,
            formal->SetType(StringAttr(ConsumeReturn())),
            "'SELF_TYPE' cannot be used in formal parameter declaration", formal),
        "expected type identifier in formal parameter declaration", formal);
    return formal;
}

Parser::ParseExprFunctor Parser::GetParseExprFunctor(Token::Type first) {
    switch (first) {
        case tok::Token::kIf:
            return &Parser::ParseIf;
        case tok::Token::kWhile:
            return &Parser::ParseWhile;
        case tok::Token::ID:
            if (MatchMultiple({Token::ID, Token::kAssignment}))
                return &Parser::ParseAssign;
            if (MatchMultiple({Token::ID, Token::kOpenParen}))
                 return &Parser::ParseCall;
            return &Parser::ParseID;
        case Token::kOpenBrace:
            return &Parser::ParseBlock;
        case Token::kOpenParen:
            return &Parser::ParseParen;
        case Token::kLet:
            return &Parser::ParseLet;
        case Token::kCase:
            return &Parser::ParseCase;
        case Token::kNew:
            return &Parser::ParseNew;
        case Token::Integer:
            return &Parser::ParseInteger;
        case Token::String:
            return &Parser::ParseString;
        case Token::kTrue:
            return &Parser::ParseTrue;
        case Token::kFalse:
            return &Parser::ParseFalse;
        default:
            return nullptr;
    }
}

Expr* Parser::OperatorExprFactory(Token::Type type, repr::Expr* left, repr::Expr* right) {
    assert (Token::IsOperator(type));

    switch (type) {
        case Token::kAdd:
            return new Add(left, right);
        case Token::kMinus:
            return new Minus(left, right);
        case Token::kMultiply:
            return new Multiply(left, right);
        case Token::kDivide:
            return new Divide(left, right);
        case Token::kLessThan:
            return new LessThan(left, right);
        case Token::kLessThanOrEqual:
            return new LessThanOrEqual(left, right);
        case Token::kDot:
            return new MethodCall(left, right);
        case Token::kEqual:
            return new Equal(left, right);
        case Token::kNegate:
            return new Negate(left);
        case Token::kNot:
            return new Not(left);
        case Token::kIsvoid:
            return new IsVoid(left);
        default:
            assert(false);
    }
}


Expr* Parser::ParseExpr() {
    vector<pair<Token, Expr*>> exprs;

    // parse unary operator into expression
    auto ParseUnary = [this](Token::Type type, int idx, vector<pair<Token, Expr*>>& exprs) {
        if (idx == exprs.size() - 1) {
            diag.EmitError(GetTextInfo(), "expected non-operator expression");
            return false;
        }
        if (exprs.at(idx + 1).first.IsOperator()) {
            diag.EmitError(exprs.at(idx + 1).first.textInfo, "expected non-operator expression");
            return false;
        }
        exprs.at(idx + 1).second = OperatorExprFactory(type, exprs.at(idx + 1).second, nullptr);
        exprs.erase(exprs.begin() + idx);
        return true;
    };

    // parse binary operator into expression
    auto ParseBinary = [this](Token::Type type, int idx, vector<pair<Token, Expr*>>& exprs) {
        if (idx == 0) {
            diag.EmitError(exprs.at(idx).first.textInfo, "expected non-operator expression");
            return false;
        }
        if (exprs.at(idx - 1).first.IsOperator()) {
            diag.EmitError(exprs.at(idx - 1).first.textInfo, "expected non-operator expression");
            return false;
        }
        if (idx == exprs.size() - 1) {
            diag.EmitError(GetTextInfo(), "expected non-operator expression");
            return false;
        }
        if (!exprs.at(idx + 1).second) {
            diag.EmitError(exprs.at(idx + 1).first.textInfo, "expected non-operator expression");
            return false;
        }

        exprs.at(idx - 1).second =
            OperatorExprFactory(type, exprs.at(idx - 1).second, exprs.at(idx + 1).second);
        exprs.erase(exprs.begin() + idx, exprs.begin() + idx + 2);
        return true;
    };

    auto ParseOperator = [ParseUnary, ParseBinary](Token::Type type, int idx, vector<pair<Token, Expr*>>& exprs) {
        assert(Token::IsOperator(type));
        if (Token::IsUnary(type)) {
            if (!ParseUnary(type, idx, exprs))
                return false;
        } else if (!ParseBinary(type, idx, exprs)) {
            return false;
        }
        return true;
    };

    auto LastOpIdx = [](vector<pair<Token, Expr*>>& exprs, int end) {
        for (int i = min(end, int(exprs.size())) - 1; i >= 0 ; --i)
            if (exprs.at(i).first.IsOperator())
                return i;
        return -1;
    };

    while (!Empty()) {

        if (!Peek().IsOperator()) { // parse operand expression

            auto functor = GetParseExprFunctor(Peek().type);
            if (!functor)
                break;
            exprs.emplace_back(Peek(), functor(*this));

        } else { // parse operator, we need to handle precedence

            auto token = ConsumeReturn();
            while (LastOpIdx(exprs, exprs.size()) >= 0) {
                int lastOpIdx = LastOpIdx(exprs, exprs.size());

                auto lastOp = exprs.at(lastOpIdx).first;

                // we are higher, cannot decide
                if (Token::GetOperatorPrecedence(token.type) > Token::GetOperatorPrecedence(lastOp.type))
                    break;

                if (!ParseOperator(lastOp.type, lastOpIdx, exprs))
                   return nullptr;
            }
            exprs.emplace_back(token, nullptr); // placeholder
        }
    }

    // todo: should first parse from right until
    //  two consecutive equal-precedence operators encountered
    while (LastOpIdx(exprs, exprs.size()) >= 0) {
        int lastOpIdx = LastOpIdx(exprs, exprs.size());

        int secondLastOpIdx = LastOpIdx(exprs, lastOpIdx);
        if (secondLastOpIdx < 0 ||
        (Token::GetOperatorPrecedence(exprs.at(lastOpIdx).first.type) <=
        Token::GetOperatorPrecedence(exprs.at(secondLastOpIdx).first.type))
        )
            break;
        if (!ParseOperator(exprs.at(lastOpIdx).first.type, lastOpIdx, exprs))
            return nullptr;
    }
    // parse remaining exprs
    for (int i = 0; i < exprs.size(); i++) {
        auto op = exprs.at(i).first;
        if (!op.IsOperator())
            continue;
        if (!ParseOperator(op.type, i, exprs))
            return nullptr;
    }

    if (exprs.empty()) {
        diag.EmitError(GetTextInfo(), "expected expression");
        return nullptr;
    }

    if (exprs.size() != 1 || exprs.front().first.IsOperator()) {
        diag.EmitError(GetTextInfo(), "invalid expression");
        return nullptr;
    }

    return exprs.front().second;
}

If* Parser::ParseIf() {
    auto anIf = new If();

    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kIf), "unexpected call to ParseIf");
    auto expr = ParseExpr();
    PARSER_CHECK_STAT_IF_FALSE_RETURN(expr, anIf->SetIfExpr(expr), anIf)

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kThen), "expected keyword 'then'", anIf);
    expr = ParseExpr();
    PARSER_CHECK_STAT_IF_FALSE_RETURN(expr, anIf->SetThenExpr(expr), anIf)

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kElse), "expected keyword 'else'", anIf);
    expr = ParseExpr();
    PARSER_CHECK_STAT_IF_FALSE_RETURN(expr, anIf->SetElseExpr(expr), anIf)

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kFi), "expected keyword 'fi'", anIf);
    return anIf;
}

Block* Parser::ParseBlock() {
    auto parse = [this]() {
        auto blk = new Block(GetTextInfo());
        vector<Expr*> exprs;

        while (!Empty()) {
            auto expr = ParseExpr();
            if (checker.Visit(expr))
                exprs.emplace_back(expr);
            else break;

            PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kSemiColon), "expected ';' after expression", blk);

//            if (Match(Token::kCloseBrace)) break;
        }

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(!exprs.empty(), "expected expression", blk);

        blk->SetExprs(exprs);
        return blk;
    };

    assert(Match(Token::kOpenBrace) && "unexpected call to ParseBlock");
    int closeBracePos = ReturnValidBraceMatchFor(Pos());
    if (closeBracePos < 0) {
        MoveTo(ScopeEnd() - 1);
        diag.EmitError(GetTextInfo(), "expected '}' in block expression");
        return nullptr;
    }
    Consume(); // consume kOpenBrace
    PushScopeEnd(closeBracePos);
    auto blk = parse();
    PopScopeEnd();
    MoveTo(closeBracePos + 1);
    return blk;
}

Expr* Parser::ParseParen() {
    int closeParenPos = ReturnValidParenMatchFor(Pos());
    if (closeParenPos < 0) {
        MoveTo(ScopeEnd() - 1);
        diag.EmitError(GetTextInfo(), "expected ')'");
        return nullptr;
    }
    Consume(); // consume OpenParen
    PushScopeEnd(closeParenPos);
    auto expr = ParseExpr();
    PopScopeEnd();
    MoveTo(closeParenPos + 1);
    return expr;
}

While* Parser::ParseWhile() {
    auto aWhile = new While();

    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kWhile), "unexpected call to ParseWhile")
    auto expr = ParseExpr();
    PARSER_CHECK_STAT_IF_FALSE_RETURN(expr, aWhile->SetWhileExpr(expr), aWhile)

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kLoop), "expected keyword 'loop'", aWhile);
    expr = ParseExpr();
    PARSER_CHECK_STAT_IF_FALSE_RETURN(expr, aWhile->SetLoopExpr(expr), aWhile)

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kPool), "expected keyword 'pool'", aWhile);
    return aWhile;
}

Let* Parser::ParseLet() {
    auto let = new Let();

    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kLet), "unexpected call to ParseLet")

    auto parseDecl = [&](){
        auto decl = new Let::Decl();

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::ID), "expected identifier", decl);
        decl->SetName(StringAttr(ConsumeReturn()));

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kColon), "expected keyword ':'", decl);

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::TypeID), "expected type identifier", decl);
        decl->SetType(StringAttr(ConsumeReturn()));

        if (ConsumeIfMatch(Token::kAssignment)) {
            auto expr = ParseExpr();
            PARSER_CHECK_STAT_IF_FALSE_RETURN(expr, decl->SetExpr(expr), decl);
        }

        return decl;
    };

    vector<Let::Decl*> decls;
    do {
        auto decl = parseDecl();
        if (checker.Visit(*decl))
            decls.emplace_back(decl);
        else return let;

    } while (ConsumeIfMatch(Token::kComma));
    let->SetDecls(decls);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kIn), "expected 'in' in let expression", let);

    auto expr = ParseExpr();
    PARSER_CHECK_STAT_IF_FALSE_RETURN(expr, let->SetExpr(expr), let);
    return let;
}

Case* Parser::ParseCase() {
    auto aCase = new Case();

    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kCase), "expected keyword 'case'")
    auto expr = ParseExpr();
    PARSER_CHECK_STAT_IF_FALSE_RETURN(expr, aCase->SetExpr(expr), aCase);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kOf), "expected 'of' in case expression", aCase);

    auto parseBranch = [&](){
        auto branch = new Case::Branch();

        PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::ID),
            branch->SetId(StringAttr(ConsumeReturn())),
            "expected identifier", branch);

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kColon), "expected ':'", branch);

        PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(
            Match(Token::TypeID),
            PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(
                Peek().val != TYPE_SELF_TYPE,
                branch->SetType(StringAttr(ConsumeReturn())),
                "'SELF_TYPE' cannot be used in case expression", branch),
            "expected type identifier", branch);

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kEval), "expected '=>'", branch);

        auto expr = ParseExpr();
        PARSER_CHECK_STAT_IF_FALSE_RETURN(expr, branch->SetExpr(expr), branch);

        PARSER_IF_FALSE_EMIT_DIAG_RETURN(ConsumeIfMatch(Token::kSemiColon), "expected ';'", branch);
        return branch;
    };

    vector<Case::Branch*> branches;
    do {
        auto branch = parseBranch();
        if (checker.Visit(*branch))
            branches.emplace_back(branch);
        else
            return aCase;
    } while (!ConsumeIfMatch(Token::kEsac));

    aCase->SetBranches(branches);

    return aCase;
}

ID* Parser::ParseID() {
    auto id = new ID();
    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::ID),
        id->SetName(StringAttr(ConsumeReturn())),
        "expected identifier", id);
    return id;
}

Assign* Parser::ParseAssign() {
    auto assign = new Assign();

    auto id = ParseID();
    PARSER_CHECK_STAT_IF_FALSE_RETURN(id, assign->SetId(id), assign)

    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kAssignment), "unexpected call to ParseAssign")

    auto expr = ParseExpr();
    PARSER_CHECK_STAT_IF_FALSE_RETURN(expr, assign->SetExpr(expr), assign);

    return assign;
}

Call* Parser::ParseCall() {
    auto call = new Call();
    PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::ID),
        call->SetId(ParseID()),
        "expected identifier in call", call);

    PARSER_IF_FALSE_EMIT_DIAG_RETURN(Match(Token::kOpenParen), "expected '(' in call expression", call)
    int end = ReturnValidParenMatchFor(Pos());
    PARSER_IF_FALSE_EMIT_DIAG_RETURN((end >= 0), "expected ')' in call expression", call);
    Consume();

    PushScopeEnd(end);
    vector<Expr*> args;
    while (!Empty()) {
        // note: if parse expr failed, should emplace nullptr so that checker can decide its invalidity
        if(!args.empty() && !ConsumeIfMatch(Token::kComma)) {
            diag.EmitError(GetTextInfo(), "expected ',' after expression");
            break;
        }
        auto expr = ParseExpr();
        if (checker.Visit(expr))
            args.emplace_back(expr);
        else break;
    }
    call->SetArgs(args);
    PopScopeEnd();
    MoveTo(end+1);
    return call;
}

New* Parser::ParseNew() {
    auto aNew = new New();
    PARSER_IF_FALSE_EXCEPTION(ConsumeIfMatch(Token::kNew), "unexpected call to ParseNew");

    // one common programing mistake is use ID instead of TypeID
    // here we consume any non-TypeID token to recover from this kind of mistake
    if (!ConsumeIfNotMatch(Token::TypeID))
        aNew->SetType(StringAttr(ConsumeReturn()));
    else
        diag.EmitError(GetTextInfo(), "expected type identifier in new expression");

    return aNew;
}

repr::Integer* Parser::ParseInteger() {
    assert(Match(Token::Integer) &&"unexpected call to ParseInteger");
    return new Integer(ConsumeReturn());
}

repr::String* Parser::ParseString() {
    assert(Match(Token::String) &&"unexpected call to ParseString");
    return new String(ConsumeReturn());
}

repr::True* Parser::ParseTrue() {
    assert(Match(Token::kTrue) && "unexpected call to ParseTrue");
    return new True(ConsumeReturn().textInfo);
}

repr::False* Parser::ParseFalse() {
    assert(Match(Token::kFalse) &&"unexpected call to ParseFalse");
    return new False(ConsumeReturn().textInfo);
}