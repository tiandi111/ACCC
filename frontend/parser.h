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

#define PARSER_CHECK_STAT_IF_FALSE_RETURN(checkee, stat, rItem) \
    if (!checker.Visit(checkee)) return rItem; \
    stat;

#define PARSER_IF_FALSE_EXCEPTION(pred, msg) if (!pred) throw runtime_error(msg);

#define PARSER_STAT_IF_FALSE_EMIT_DIAG_SKIP(pred, stat, msg) \
    if (pred) stat; \
    else { \
        diag.EmitError(GetTextInfo(), msg); \
        Consume(); \
    } \

#define PARSER_STAT_IF_FALSE_EMIT_DIAG_RETURN(pred, stat, msg, rItem) \
    if (pred) {stat;} \
    else { \
        diag.EmitError(GetTextInfo(), msg); \
        return rItem; \
    } \

#define PARSER_IF_FALSE_EMIT_DIAG_RETURN(pred, msg, rItem) \
    if (!pred) {\
        diag.EmitError(GetTextInfo(), msg);\
        return rItem;\
    }\

using namespace visitor;
using namespace tok;

class ParsingResultChecker : public ExprVisitor<bool> {
  public:
    bool Visit(repr::Program &prog);
    bool Visit(repr::Class &cls);
    bool Visit(repr::FuncFeature &feat);
    bool Visit(repr::FieldFeature &feat);
    bool Visit(repr::Formal &form);
    bool Visit(repr::Expr* expr);
    bool Visit_(repr::LinkBuiltin& expr);
    bool Visit_(repr::Assign& expr);
    bool Visit_(repr::Add& expr);
    bool Visit_(repr::Block& expr);
    bool VisitBinary(repr::Binary& expr);
    bool Visit_(repr::Case& expr);
    bool Visit(repr::Case::Branch& branch);
    bool Visit_(repr::Call& expr);
    bool Visit_(repr::Divide& expr);
    bool Visit_(repr::Equal& expr);
    bool Visit_(repr::False& expr);
    bool Visit_(repr::ID& expr);
    bool Visit_(repr::IsVoid& expr);
    bool Visit_(repr::Integer& expr);
    bool Visit_(repr::If& expr);
    bool Visit_(repr::LessThanOrEqual& expr);
    bool Visit_(repr::LessThan& expr);
    bool Visit_(repr::Let& expr);
    bool Visit(repr::Let::Decl& expr);
    bool Visit_(repr::MethodCall& expr);
    bool Visit_(repr::Multiply& expr);
    bool Visit_(repr::Minus& expr);
    bool Visit_(repr::Negate& expr);
    bool Visit_(repr::New& expr);
    bool Visit_(repr::Not& expr);
    bool Visit_(repr::String& expr);
    bool Visit_(repr::True& expr);
    bool Visit_(repr::While& expr);
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

    diag::TextInfo GetTextInfo();

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

    repr::Program* ParseProgram();

    repr::Class* ParseClass();

    repr::FuncFeature* ParseFuncFeature();
    repr::FieldFeature* ParseFieldFeature();

    repr::Formal* ParseFormal();

    using ParseExprFunctor = function<repr::Expr* (Parser&)>;

    ParseExprFunctor GetParseExprFunctor(Token::Type);

    repr::Expr* OperatorExprFactory(Token::Type, repr::Expr*, repr::Expr*);

    repr::Expr* ParseExpr();
    repr::If* ParseIf();
    repr::Block* ParseBlock();
    repr::Expr* ParseParen();
    repr::While* ParseWhile();
    repr::Let* ParseLet();
    repr::Case* ParseCase();
    repr::ID* ParseID();
    repr::Assign* ParseAssign();
    repr::Call* ParseCall();
    repr::New* ParseNew();
    repr::Integer* ParseInteger();
    repr::String* ParseString();
    repr::True* ParseTrue();
    repr::False* ParseFalse();
};

} // parser

} // cool

#endif //COOL_PARSER_H
