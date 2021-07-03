//
// Created by 田地 on 2021/5/30.
//

#ifndef C_PARSER_H
#define C_PARSER_H

#include <string>
#include <utility>
#include <vector>
#include <memory>

#include "token.h"
#include "diag.h"

using namespace std;

namespace cool {
    
namespace repr {

struct Attr {
    diag::TextInfo textInfo;

    Attr(int _line = -1, int _pos = -1, int _fileno = -1) : textInfo({_line, _pos, _fileno}) {}

    Attr(diag::TextInfo _textInfo) : textInfo(_textInfo) {}

    Attr(tok::Token&& token) : textInfo(token.textInfo) {}
};

struct StringAttr : Attr {
    string val;

    StringAttr() = default;

    StringAttr(const string& _val, int _line = -1, int _pos = -1, int _fileno = -1)
    : val(_val), Attr(_line, _pos, _fileno) {}

    StringAttr(tok::Token&& token) : val(token.str), Attr(token.textInfo) {}

    bool Empty() { return val.empty(); }
};

struct IntAttr : Attr {
    int val;

    IntAttr() = default;

    IntAttr(int _val, int _line = -1, int _pos = -1, int _fileno = -1)
    : val(_val), Attr(_line, _pos, _fileno) {}

    IntAttr(tok::Token&& token) : Attr(token.textInfo) {
        if (token.type != tok::Token::Integer) throw runtime_error("");
        val = stoi(token.str);
    }
};

struct Repr {
    virtual ~Repr() = default;
    virtual diag::TextInfo GetTextInfo() = 0;
};

struct Formal : Repr {
    StringAttr name;
    StringAttr type;
    Formal() = default;
    Formal(StringAttr&& _name, StringAttr&& _type) : name(_name), type(_type) {}
    inline diag::TextInfo GetTextInfo() final { return name.textInfo; }
};

struct Expr : Repr {
    virtual ~Expr() = default;
};

struct LinkBuiltin : public Expr {
    string type;
    LinkBuiltin(const string& _type) : type(_type) {}
    inline diag::TextInfo GetTextInfo() final { return diag::TextInfo{}; }
};

struct ID : public Expr {
    StringAttr name;
    ID() = default;
    ID(StringAttr& _name) : name(_name) {}
    inline diag::TextInfo GetTextInfo() final { return name.textInfo; }
};

struct Assign : public Expr {
    shared_ptr<ID> id;
    shared_ptr<Expr> expr;
    Assign() = default;
    inline diag::TextInfo GetTextInfo() final { return id->GetTextInfo(); }
};

struct Call : public Expr {
    shared_ptr<ID> id;
    vector<shared_ptr<Expr>> args;
    Call() = default;
    inline diag::TextInfo GetTextInfo() final { return id->GetTextInfo(); }
};

struct If : public Expr {
    shared_ptr<Expr> ifExpr;
    shared_ptr<Expr> thenExpr;
    shared_ptr<Expr> elseExpr;
    If() = default;
    inline diag::TextInfo GetTextInfo() final { return ifExpr->GetTextInfo(); }
};

struct Block : public Expr {
    diag::TextInfo textInfo;
    vector<shared_ptr<Expr>> exprs;
    Block(diag::TextInfo _textInfo) : textInfo(_textInfo) {}
    inline diag::TextInfo GetTextInfo() final { return textInfo; }
};

struct While : public Expr {
    shared_ptr<Expr> whileExpr;
    shared_ptr<Expr> loopExpr;
    inline diag::TextInfo GetTextInfo() final { return whileExpr->GetTextInfo(); }
};

struct Let : public Expr {
    struct Formal : Repr {
        StringAttr name;
        StringAttr type;
        shared_ptr<Expr> expr;
        diag::TextInfo GetTextInfo() final { return name.textInfo; }
    };
    vector<shared_ptr<Let::Formal>> formals;
    shared_ptr<Expr> expr;
    inline diag::TextInfo GetTextInfo() final { return expr->GetTextInfo(); }
};

struct Case : public Expr {
    struct Branch {
        StringAttr id;
        StringAttr type;
        shared_ptr<Expr> expr;
    };
    shared_ptr<Expr> expr;
    vector<shared_ptr<Branch>> branches;
    inline diag::TextInfo GetTextInfo() final { return expr->GetTextInfo(); }
};

struct New : public Expr {
    StringAttr type;
    inline diag::TextInfo GetTextInfo() final { return type.textInfo; }
};

// note: only provide constructor by pointer because we hope to pertain the dynamic type
struct Unary : public Expr {
    shared_ptr<Expr> expr;
    Unary(shared_ptr<Expr> _expr) : expr(move(_expr)) {}
    inline diag::TextInfo GetTextInfo() final { return expr->GetTextInfo(); }
};

struct IsVoid : public Unary {
    IsVoid(shared_ptr<Expr> _expr) : Unary(move(_expr)) {}
};

struct Negate : Unary {
    Negate(shared_ptr<Expr> _expr) : Unary(move(_expr)) {}
};

struct Not : public Unary {
    Not(shared_ptr<Expr> _expr) : Unary(move(_expr)) {}
};

struct Binary : public Expr {
    shared_ptr<Expr> left;
    shared_ptr<Expr> right;
    Binary(shared_ptr<Expr> _left, shared_ptr<Expr> _right) : left(move(_left)), right(move(_right)) {}
    inline diag::TextInfo GetTextInfo() final { return left->GetTextInfo(); }
};

struct Add : public Binary {
    Add(shared_ptr<Expr> _left, shared_ptr<Expr> _right) : Binary(move(_left), move(_right)) {}
};

struct Minus : public Binary {
    Minus(shared_ptr<Expr> _left, shared_ptr<Expr> _right) : Binary(move(_left), move(_right)) {}
};

struct Multiply : public Binary {
    Multiply(shared_ptr<Expr> _left, shared_ptr<Expr> _right) : Binary(move(_left), move(_right)) {}
};

struct Divide : public Binary {
    Divide(shared_ptr<Expr> _left, shared_ptr<Expr> _right) : Binary(move(_left), move(_right)) {}
};

struct LessThan : public Binary {
    LessThan(shared_ptr<Expr> _left, shared_ptr<Expr> _right) : Binary(move(_left), move(_right)) {}
};

struct LessThanOrEqual : public Binary {
    LessThanOrEqual(shared_ptr<Expr> _left, shared_ptr<Expr> _right) : Binary(move(_left), move(_right)) {}
};

struct Equal : public Binary {
    Equal(shared_ptr<Expr> _left, shared_ptr<Expr> _right) : Binary(move(_left), move(_right)) {}
};

struct MethodCall : public Binary {
    MethodCall(shared_ptr<Expr> _left, shared_ptr<Expr> _right) : Binary(move(_left), move(_right)) {}
};

struct Integer : public Expr {
    IntAttr val;
    Integer(IntAttr&& _val) : val(_val) {}
    inline diag::TextInfo GetTextInfo() final { return val.textInfo; }
};

struct String : public Expr {
    StringAttr val;
    String(StringAttr&& _val) : val(_val) {}
    inline diag::TextInfo GetTextInfo() final { return val.textInfo; }
};

struct True : public Expr {
    diag::TextInfo textInfo;
    True(diag::TextInfo _textInfo) : textInfo(_textInfo) {}
    inline diag::TextInfo GetTextInfo() final { return textInfo; }
};

struct False : public Expr {
    diag::TextInfo textInfo;
    False(diag::TextInfo _textInfo) : textInfo(_textInfo) {}
    inline diag::TextInfo GetTextInfo() final { return textInfo; }
};

struct FuncFeature : Repr {
    StringAttr name;
    StringAttr type;
    shared_ptr<Expr> expr;
    vector<shared_ptr<Formal>> args;

    FuncFeature() = default;
    FuncFeature(StringAttr _name, StringAttr _type, shared_ptr<Expr> _expr, vector<shared_ptr<Formal>> _args)
    : name(_name), type(_type), expr(_expr), args(_args) {}
    inline diag::TextInfo GetTextInfo() final { return name.textInfo; }
};

struct FieldFeature : Repr {
    StringAttr name;
    StringAttr type;
    shared_ptr<Expr> expr;
    inline diag::TextInfo GetTextInfo() final { return name.textInfo; }
};

struct Class : Repr {
    StringAttr name;
    StringAttr parent;
    vector<shared_ptr<FuncFeature>> funcs;
    vector<shared_ptr<FieldFeature>> fields;

    Class() = default;
    Class(StringAttr _name, StringAttr _parent, vector<shared_ptr<FuncFeature>> _funcs,
        vector<shared_ptr<FieldFeature>> _fields) : name(_name), parent(_parent), funcs(_funcs), fields(_fields) {}
    inline diag::TextInfo GetTextInfo() final { return name.textInfo; }
};

struct Program : Repr {
    diag::TextInfo textInfo;
    vector<shared_ptr<Class>> classes;

    Program(diag::TextInfo _textInfo) : textInfo(_textInfo) {}
    inline diag::TextInfo GetTextInfo() final { return textInfo; }
};

} // expr

} // cool

#endif //C_PARSER_H
