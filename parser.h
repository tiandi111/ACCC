//
// Created by 田地 on 2021/5/30.
//

#ifndef COOL_PARSER_H
#define COOL_PARSER_H

#include <utility>
#include <vector>

#include "repr.h"
#include "token.h"
#include "diag.h"

using namespace std;

namespace cool {

namespace parser {

#define ConsumeIfMatchWithException(tokType, msg) \
    if (!ConsumeIfMatch(tokType)) { \
        throw runtime_error(msg);\
    }\

#define AssignIfMatchWithException(tokType, assign, msg) \
    if (Match(tokType)) {\
        assign;\
    } else {\
        throw runtime_error(msg);\
    }\

class Parser {
  private:
    diag::Diagnosis& diag;
    vector<Token> toks;
    int pos;

  public:
    Parser(diag::Diagnosis& _diag, vector<Token> _toks) : diag(_diag), toks(std::move(_toks)), pos(0) {}

    Token& Peek();

    int Empty();

    int Line();

    void Consume();

    Token ConsumeReturn();

    bool ConsumeIfMatch(Token::Type type);

    bool Match(Token::Type type);

    bool MatchMultiple(vector<Token::Type> types);

    template<typename T>
    vector<T> MatchSequence(Token::Type start, Token::Type sep, Token::Type end, function<T()> f);

    // todo: is it good to return share_ptr here?
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
