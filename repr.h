//
// Created by 田地 on 2021/5/30.
//

#ifndef C_PARSER_H
#define C_PARSER_H

#include <string>
#include <utility>
#include <vector>
#include <memory>

using namespace std;

namespace cool {
    
namespace repr {

struct Formal {
    string name;
    string type;
};

struct Expr {
    virtual ~Expr() = default;
};

struct ID : public Expr {
    string name;
};

struct Assign : public Expr {
    shared_ptr<ID> id;
    shared_ptr<Expr> expr;
};

struct Call : public Expr {
    shared_ptr<ID> id;
    vector<shared_ptr<Expr>> args;
};

struct If : public Expr {
    shared_ptr<Expr> ifExpr;
    shared_ptr<Expr> thenExpr;
    shared_ptr<Expr> elseExpr;
};

struct Block : public Expr {
    vector<shared_ptr<Expr>> exprs;
};


struct While : public Expr {
    shared_ptr<Expr> whileExpr;
    shared_ptr<Expr> loopExpr;
};

struct Let : public Expr {
    struct Formal {
        string name;
        string type;
        shared_ptr<Expr> expr;
    };
    vector<shared_ptr<Let::Formal>> formals;
    shared_ptr<Expr> expr;
};

struct Case : public Expr {
    struct Branch {
        string id;
        string type;
        shared_ptr<Expr> expr;
    };
    shared_ptr<Expr> expr;
    vector<shared_ptr<Branch>> branches;
};

struct New : public Expr {
    string type;
};

// note: only provide constructor by pointer because we hope to pertain the dynamic type
struct Unary : public Expr {
    shared_ptr<Expr> expr;

    Unary(shared_ptr<Expr> _expr) : expr(move(_expr)) {}
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

struct Integer : public Expr {};

struct String : public Expr {};

struct True : public Expr {};

struct False : public Expr {};

struct FuncFeature {
    string name;
    string type;
    shared_ptr<Expr> expr;
    vector<shared_ptr<Formal>> args;
};

struct FieldFeature {
    string name;
    string type;
    shared_ptr<Expr> expr;
};

struct Class {
    string name;
    string parent;
    vector<shared_ptr<FuncFeature>> funcs;
    vector<shared_ptr<FieldFeature>> fields;
};

struct Program {
    vector<shared_ptr<Class>> classes;
};

} // expr

} // cool

#endif //C_PARSER_H
