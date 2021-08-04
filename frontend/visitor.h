//
// Created by 田地 on 2021/6/17.
//

#ifndef COOL_VISITOR_H
#define COOL_VISITOR_H

#include <iostream>

#include "repr.h"
#include "adt.h"
#include "vtable.h"

using namespace std;

namespace cool {

namespace visitor {

template<typename R, typename... Args>
class ProgramVisitor {
  public:
    virtual R Visit(repr::Program &prog, Args... args) { throw "Visit not defined"; }
    virtual R Visit(repr::Program* prog, Args... args) { throw "Visit not defined"; }
};

template<typename R, typename... Args>
class ClassVisitor {
  public:
    virtual R Visit(repr::Class &cls, Args... args) { throw "Visit not defined"; }
    virtual R Visit(repr::Class* ptr, Args... args) { throw "Visit not defined"; }
};

template<typename R, typename... Args>
class FuncFeatureVisitor {
  public:
    virtual R Visit(repr::FuncFeature &feat, Args... args) { throw "Visit not defined"; }
    virtual R Visit(repr::FuncFeature* feat, Args... args) { throw "Visit not defined"; }
};

template<typename R, typename... Args>
class FieldFeatureVisitor {
  public:
    virtual R Visit(repr::FieldFeature &feat, Args... args) { throw "Visit not defined"; }
    virtual R Visit(repr::FieldFeature* feat, Args... args) { throw "Visit not defined"; }
};

template<typename R, typename... Args>
class FormalVisitor {
  public:
    virtual R Visit(repr::Formal &form, Args... args) { throw "Visit not defined"; }
    virtual R Visit(repr::Formal* form, Args... args) { throw "Visit not defined"; }
};

template<typename R, typename... Args>
class ExprVisitor {
  private:
    using Self = ExprVisitor;
    using VTable = VirtualTable<R, repr::Expr, Self&, Args...>;

  public:
    R Visit(repr::Expr& expr, Args... args) {
        VTable vtable = GetVTable();
        return vtable(expr, *this, args...);
    }

    #define EXPR_VISITOR_DEFAULT { VisitDefault_(); }
    virtual R Visit_(repr::LinkBuiltin& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::Assign& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::Add& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::Block& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::Case& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::Call& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::Divide& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::Equal& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::False& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::ID& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::IsVoid& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::Integer& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::If& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::LessThanOrEqual& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::LessThan& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::Let& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::MethodCall& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::Multiply& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::Minus& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::Negate& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::New& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::Not& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::String& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::True& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R Visit_(repr::While& expr, Args... args) EXPR_VISITOR_DEFAULT;
    virtual R VisitDefault_() {
        cerr<< "default not found" <<endl;
        throw;
    }

  private:
    #define EXPR_VISITOR_DISPATCH(Class)\
        vtable.template SetDispatch<Class>([](repr::Expr& expr, Self& self, Args... args){\
            return self.Visit_(static_cast<Class&>(expr));\
        })\

    static VTable GetVTable() {
        static VTable vtable;
        EXPR_VISITOR_DISPATCH(repr::LinkBuiltin);
        EXPR_VISITOR_DISPATCH(repr::Assign);
        EXPR_VISITOR_DISPATCH(repr::Add);
        EXPR_VISITOR_DISPATCH(repr::Block);
        EXPR_VISITOR_DISPATCH(repr::Case);
        EXPR_VISITOR_DISPATCH(repr::Call);
        EXPR_VISITOR_DISPATCH(repr::Divide);
        EXPR_VISITOR_DISPATCH(repr::Equal);
        EXPR_VISITOR_DISPATCH(repr::False);
        EXPR_VISITOR_DISPATCH(repr::ID);
        EXPR_VISITOR_DISPATCH(repr::IsVoid);
        EXPR_VISITOR_DISPATCH(repr::Integer);
        EXPR_VISITOR_DISPATCH(repr::If);
        EXPR_VISITOR_DISPATCH(repr::LessThanOrEqual);
        EXPR_VISITOR_DISPATCH(repr::LessThan);
        EXPR_VISITOR_DISPATCH(repr::Let);
        EXPR_VISITOR_DISPATCH(repr::MethodCall);
        EXPR_VISITOR_DISPATCH(repr::Multiply);
        EXPR_VISITOR_DISPATCH(repr::Minus);
        EXPR_VISITOR_DISPATCH(repr::Negate);
        EXPR_VISITOR_DISPATCH(repr::New);
        EXPR_VISITOR_DISPATCH(repr::Not);
        EXPR_VISITOR_DISPATCH(repr::String);
        EXPR_VISITOR_DISPATCH(repr::True);
        EXPR_VISITOR_DISPATCH(repr::While);
        return vtable;
    }
};

} // visitor

} // cool

#endif //COOL_VISITOR_H
