//
// Created by 田地 on 2021/5/30.
//

#ifndef COOL_PARSER_H
#define COOL_PARSER_H

#include <utility>
#include <vector>
#include <unordered_set>
#include <stack>

#include "repr.h"
#include "token.h"
#include "diag.h"
#include "visitor.h"

using namespace std;

namespace cool {

namespace parser {

#define PARSER_CHECK_ASSGIN_RETURN(left, right, rItem) \
    if (!checker.Visit(right)) return rItem; \
    left = right;

#define PARSER_IF_FALSE_EXCEPTION(pred, msg) if (!pred) throw runtime_error(msg);

#define PARSER_STAT_IF_FALSE_EMIT_DIAG_SKIP(pred, stat, msg) \
    if (pred) stat; \
    else { \
        diag.EmitError(FileName(), TextLine(), TextPos(), msg); \
        Consume(); \
    } \

#define PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(pred, stat, msg, rItem) \
    if (pred) stat; \
    else { \
        diag.EmitError(FileName(), TextLine(), TextPos(), msg); \
        return rItem; \
    } \

#define PARSER_IF_FALSE_EMIT_DIAG_RETURN(pred, msg, rItem) \
    if (!pred) {\
        diag.EmitError(FileName(), TextLine(), TextPos(), msg);\
        return rItem;\
    }\

#define AssignIfMatchWithException(tokType, assign, msg) \
    if (Match(tokType)) {\
        assign;\
    } else {\
        throw runtime_error(msg);\
    }\

using namespace visitor;
using namespace tok;

class ParsingResultChecker : public ExprVisitor<bool> {
  public:
    bool Visit(repr::Program &prog) { return true; }
    bool Visit(repr::Class &cls) { return true;}
    bool Visit(repr::FuncFeature &feat) { return true; }
    bool Visit(repr::FieldFeature &feat) { return true; }
    bool Visit(repr::Formal &form) { return true; }
    bool Visit(repr::Let::Formal &form) { return true; }
    bool Visit(repr::Expr& expr) { return ExprVisitor<bool>::Visit(expr); }
    bool Visit(shared_ptr<repr::Expr>& expr) { return ExprVisitor<bool>::Visit(*expr); }
    bool Visit_(repr::Assign& expr) { return true; };
    bool Visit_(repr::Add& expr) { return true; };
    bool Visit_(repr::Block& expr) { return true; };
    bool Visit_(repr::Case& expr) { return true; };
    bool Visit(repr::Case::Branch& branch) { return true; };
    bool Visit_(repr::Call& expr) { return true; };
    bool Visit_(repr::Divide& expr) { return true; };
    bool Visit_(repr::Equal& expr) { return true; };
    bool Visit_(repr::False& expr) { return true; };
    bool Visit_(repr::ID& expr) { return true; };
    bool Visit_(repr::IsVoid& expr) { return true; };
    bool Visit_(repr::Integer& expr) { return true; };
    bool Visit_(repr::If& expr) { return true; };
    bool Visit_(repr::LessThanOrEqual& expr) { return true; };
    bool Visit_(repr::LessThan& expr) { return true; };
    bool Visit_(repr::Let& expr) { return true; };
    bool Visit_(repr::MethodCall& expr) { return true; };
    bool Visit_(repr::Multiply& expr) { return true; };
    bool Visit_(repr::Minus& expr) { return true; };
    bool Visit_(repr::Negate& expr) { return true; };
    bool Visit_(repr::New& expr) { return true; };
    bool Visit_(repr::Not& expr) { return true; };
    bool Visit_(repr::String& expr) { return true; };
    bool Visit_(repr::True& expr) { return true; };
    bool Visit_(repr::While& expr) { return true; };
};

class Parser {
  private:
    ParsingResultChecker checker;
    diag::Diagnosis& diag;
    vector<Token> toks;
    stack<int> scopeEnds;
    int pos;

  public:
    Parser(diag::Diagnosis& _diag, vector<Token> _toks);

    Token& Peek();

    bool Empty();

    int Pos();

    int TextLine();

    int TextPos();

    string FileName();

    void Consume();

    Token ConsumeReturn();

    bool ConsumeIfMatch(Token::Type type);

    bool ConsumeIfNotMatch(Token::Type type);

    bool Match(Token::Type type);

    bool MatchMultiple(vector<Token::Type> types);

    bool SkipTo(unordered_set<Token::Type> types);

    void PushScopeEnd(int scopeEnd);

    int PopScopeEnd();

    int ReturnNextPos(Token::Type type);

    int ReturnValidParenMatchFor(int p);

    int ReturnValidBraceMatchFor(int p);

    void MoveTo(int cur);

    int ScopeEnd();

    repr::Program ParseProgram();

    repr::Class ParseClass();

    repr::FuncFeature ParseFuncFeature();
    repr::FieldFeature ParseFieldFeature();

    repr::Formal ParseFormal();

    shared_ptr<repr::Expr> ParseExpr();
    repr::If ParseIf();
    repr::Block ParseBlock();
    repr::While ParseWhile();
    repr::Let ParseLet();
    repr::Case ParseCase();
    repr::ID ParseID();
    repr::Assign ParseAssign();
    repr::Call ParseCall();
    repr::New ParseNew();
    repr::Integer ParseInteger();
    repr::String ParseString();
    repr::True ParseTrue();
    repr::False ParseFalse();

    repr::IsVoid ParseIsVoid();
    repr::Negate ParseNegate();
    repr::Not ParseNot();

    repr::Add ParseAdd(shared_ptr<repr::Expr> left);
    repr::Minus ParseMinus(shared_ptr<repr::Expr> left);
    repr::Multiply ParseMultiply(shared_ptr<repr::Expr>left);
    repr::Divide ParseDivide(shared_ptr<repr::Expr> left);
    repr::LessThan ParseLessThan(shared_ptr<repr::Expr> left);
    repr::LessThanOrEqual ParseLessThanOrEqual(shared_ptr<repr::Expr> left);
    repr::Equal ParseEqual(shared_ptr<repr::Expr> left);
    repr::MethodCall ParseMethodCall(shared_ptr<repr::Expr> left);
};

} // parser

} // cool

#endif //COOL_PARSER_H
