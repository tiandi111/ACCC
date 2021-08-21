//
// Created by 田地 on 2021/5/30.
//

#ifndef C_PARSER_H
#define C_PARSER_H

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <iostream>

#include "token.h"
#include "diag.h"

using namespace std;

namespace cool {
    
namespace repr {

//======================================================================//
// Lesson Learned:                                                      //
// Provide copy constructor when there're dynamic memory allocations.   //
// In other cases, the default constructor is enough.                   //
//======================================================================//

//======================================================================//
// Lesson Learned:                                                      //
// Methods defined in class definition is 'inlined' as default.         //
//======================================================================//

//======================================================================//
// Lesson Learned:                                                      //
// Virtual functions with return type as shared_ptr<Base> cannot be     //
// override by functions with return type as shared_ptr<Derived>.       //
// This is called "Covariant Return Type" in C++. Raw pointers are      //
// covariant.                                                           //
//======================================================================//

//======================================================================//
//                             Attr  Class                              //
//======================================================================//
class Attr {
  private:
    diag::TextInfo textInfo;

  public:
    Attr(int _line = -1, int _pos = -1, int _fileno = -1)
    : textInfo({_line, _pos, _fileno}) {}

    Attr(const diag::TextInfo& _textInfo) : textInfo(_textInfo) {}

    Attr(const tok::Token& token) : textInfo(token.textInfo) {}

    diag::TextInfo TextInfo() const { return textInfo; }
};

//======================================================================//
//                         StringAttr  Class                            //
//======================================================================//
class StringAttr : public Attr {
  private:
    string val;

  public:
    StringAttr() = default;

    StringAttr(const string& _val, int _line = -1,
        int _pos = -1,int _fileno = -1)
    : val(_val), Attr(_line, _pos, _fileno) {}

    StringAttr(const tok::Token& token)
    : val(token.str), Attr(token.textInfo) {}

    StringAttr(const string& _val, const diag::TextInfo& _textInfo)
    : val(_val), Attr(_textInfo) {}

    string Value() { return val; }

    void SetValue(const string& _val) { val = _val; }

    bool Empty() const { return val.empty(); }
};

//======================================================================//
//                          IntAttr  Class                              //
//======================================================================//
class IntAttr : public Attr {
  private:
    int val;

  public:
    IntAttr() = default;

    IntAttr(int _val, int _line = -1, int _pos = -1, int _fileno = -1)
    : val(_val), Attr(_line, _pos, _fileno) {}

    IntAttr(const tok::Token& token) : Attr(token.textInfo) {
        assert ((token.type == tok::Token::Integer)
        && "token type must be Integer");
        val = stoi(token.str);
    }

    IntAttr(int _val, const diag::TextInfo& _textInfo)
    : val(_val), Attr(_textInfo) {}

    int Value() const { return val; }

    void SetValue(int _val) { val = _val; }
};

#define COOL_REPR_SETTER_GETTER(Type, Name, Field)\
    Type Get##Name() const { return Field; }\
    void Set##Name(const Type& _##Field) { Field = _##Field; }

#define COOL_REPR_SETTER_GETTER_POINTER(Type, Name, Field)\
    Type* Get##Name() { return Field; }\
    void Set##Name(Type* _##Field) { Field = _##Field; }

#define COOL_REPR_BASE_CONSTRUCTOR(Type)\
    Type() = default;\
    Type(const Type&) = delete;\
    Type& operator=(const Type&) = delete;\

//======================================================================//
// Lesson Learned:                                                      //
// Templates are all about the compiler generating code at compile-time.//
// Virtual functions are all about the run-time system figuring out     //
// which function to call at run-time.                                  //
//======================================================================//

//======================================================================//
// Lesson Learned:                                                      //
// It is necessary to differentiate owning pointer and non-owning pointer/
// Owning pointer is a pointer that needs to be deleted by its owner.   //
// For owning pointer, it is better to use smart pointer to manage its  //
// life cycle. For non-owning pointer, raw pointer is okay.             //
//======================================================================//

//======================================================================//
//                             Repr  Class                              //
//======================================================================//
// todo: attach original token with repr
class Repr {
  public:
    virtual ~Repr() = default;
    virtual diag::TextInfo GetTextInfo() const = 0;
    virtual Repr* Clone() = 0;
};


//======================================================================//
//                           Expr  Class                                //
//======================================================================//
class Expr : public Repr {
  public:
    virtual ~Expr() = default;
    virtual Expr* Clone() = 0;
};

//======================================================================//
//                           Formal  Class                              //
//======================================================================//
class Formal : public Repr {
  private:
    StringAttr name;
    StringAttr type;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Formal)

    Formal(const StringAttr& _name, const StringAttr& _type)
    : name(_name), type(_type) {}

    Formal* Clone() final { return new Formal(name, type); }

    diag::TextInfo GetTextInfo() const final { return name.TextInfo(); }

    COOL_REPR_SETTER_GETTER(StringAttr, Name, name)
    COOL_REPR_SETTER_GETTER(StringAttr, Type, type)
};

//======================================================================//
//                        LinkBuiltin  Class                            //
//======================================================================//
class LinkBuiltin : public Expr {
  private:
    string name;
    string type;
    vector<string> params;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(LinkBuiltin)

    LinkBuiltin(const string& _name, const string& _type,
        const vector<string>& _params)
    : name(_name), type(_type), params(move(_params)) {}

    LinkBuiltin* Clone() final {
        return new LinkBuiltin(name, type, params);
    }

    diag::TextInfo GetTextInfo() const final {
        return diag::TextInfo{};
    }

    COOL_REPR_SETTER_GETTER(string, Name, name)
    COOL_REPR_SETTER_GETTER(string, Type, type)
    COOL_REPR_SETTER_GETTER(vector<string>, Params, params)
};

//======================================================================//
//                             ID  Class                                //
//======================================================================//
class ID : public Expr {
  private:
    StringAttr name;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(ID)

    ID(const StringAttr& _name) : name(_name) {}

    ID* Clone() final {
        return new ID(name);
    }

    diag::TextInfo GetTextInfo() const final {
        return name.TextInfo();
    }

    COOL_REPR_SETTER_GETTER(StringAttr, Name, name)
};

//======================================================================//
//                          Assign  Class                               //
//======================================================================//
class Assign : public Expr {
  private:
    ID* id;
    Expr* expr;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Assign)

    Assign(ID* _id, Expr* _expr) : id(_id), expr(_expr) {}

    Assign* Clone() final {
        return new Assign(id->Clone(), expr->Clone());
    }

    diag::TextInfo GetTextInfo() const final { return id->GetTextInfo(); }

    COOL_REPR_SETTER_GETTER_POINTER(ID, Id, id)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
};

//======================================================================//
//                           Call  Class                                //
//======================================================================//
class FuncFeature;
class Call : public Expr {
  private:
    ID* id;
    vector<Expr*> args;
    FuncFeature* link;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Call)

    Call(ID* _id, const vector<Expr*>& _args, FuncFeature* _link)
    : id(_id), args(_args), link(_link) {}

    Call* Clone() final { return new Call(id, args, link);  }

    diag::TextInfo GetTextInfo() const final { return id->GetTextInfo(); }

    COOL_REPR_SETTER_GETTER_POINTER(ID, Id, id)
    COOL_REPR_SETTER_GETTER(vector<Expr*>, Args, args)
    COOL_REPR_SETTER_GETTER_POINTER(FuncFeature, Link, link)
};

//======================================================================//
//                              If Class                                //
//======================================================================//
class If : public Expr {
  private:
    Expr* ifExpr;
    Expr* thenExpr;
    Expr* elseExpr;
    string type;

public:
    COOL_REPR_BASE_CONSTRUCTOR(If)

    If(Expr* _ifExpr, Expr* _thenExpr, Expr* _elseExpr)
    : ifExpr(_ifExpr), thenExpr(_thenExpr), elseExpr(_elseExpr) {}

    If* Clone() final {
        return new If(
            ifExpr->Clone(),
            thenExpr->Clone(),
            elseExpr->Clone());
    }

    diag::TextInfo GetTextInfo() const final {
        return ifExpr->GetTextInfo();
    }

    COOL_REPR_SETTER_GETTER_POINTER(Expr, IfExpr, ifExpr)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, ThenExpr, thenExpr)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, ElseExpr, elseExpr)
    COOL_REPR_SETTER_GETTER(string, Type, type)
};

//======================================================================//
//                           Block Class                                //
//======================================================================//
class Block : public Expr {
  private:
    diag::TextInfo textInfo;
    vector<Expr*> exprs;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Block)

    Block(diag::TextInfo _textInfo, const vector<Expr*>& _exprs = {})
    : textInfo(_textInfo), exprs(_exprs) {}

    Block* Clone() final {
        return new Block(textInfo, exprs);
    }

    diag::TextInfo GetTextInfo() const final { return textInfo; }

    COOL_REPR_SETTER_GETTER(vector<Expr*>, Exprs, exprs)
};

//======================================================================//
//                            While Class                               //
//======================================================================//
class While : public Expr {
  private:
    Expr* whileExpr;
    Expr* loopExpr;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(While)

    While(Expr* _whileExpr, Expr* _loopExpr)
    : whileExpr(_whileExpr), loopExpr(_loopExpr) {}

    While* Clone() final {
        new While(whileExpr->Clone(),
            loopExpr->Clone());
    }

    diag::TextInfo GetTextInfo() const final {
        return whileExpr->GetTextInfo();
    }

    COOL_REPR_SETTER_GETTER_POINTER(Expr, WhileExpr, whileExpr)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, LoopExpr, loopExpr)
};

//======================================================================//
//                             Let Class                                //
//======================================================================//
class Let : public Expr {
  public:
    class Decl : public Repr {
      private:
        StringAttr name;
        StringAttr type;
        Expr* expr;

      public:
        COOL_REPR_BASE_CONSTRUCTOR(Decl)

        Decl(const StringAttr& _name, const StringAttr& _type,
            Expr* _expr)
            : name(_name), type(_type), expr(_expr) {}

        Decl* Clone() final {
            return new Decl(name, type, expr->Clone());
        }

        diag::TextInfo GetTextInfo() const final {
            return name.TextInfo();
        }

        COOL_REPR_SETTER_GETTER(StringAttr, Name, name)
        COOL_REPR_SETTER_GETTER(StringAttr, Type, type)
        COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
    };

  private:
    vector<Let::Decl*> decls;
    Expr* expr;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Let)

    Let(const vector<Let::Decl*>& _decls, Expr* _expr)
    : decls(_decls), expr(_expr) {}

    Let* Clone() final {
        return new Let(decls, expr->Clone());
    }

    diag::TextInfo GetTextInfo() const final {
        return expr->GetTextInfo();
    }

    COOL_REPR_SETTER_GETTER(vector<Let::Decl*>, Decls, decls)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
};

//======================================================================//
//                           Case Class                                 //
//======================================================================//
class Case : public Expr {
  public:
    class Branch : public Repr {
      private:
        StringAttr id;
        StringAttr type;
        Expr* expr;

      public:
        COOL_REPR_BASE_CONSTRUCTOR(Branch)

        Branch(const StringAttr& _id, const StringAttr& _type,
            Expr* _expr)
        : id(_id), type(_type), expr(_expr) {}

        Branch* Clone() final {
            return new Branch(id, type, expr->Clone());
        }

        diag::TextInfo GetTextInfo() const final {
            return expr->GetTextInfo();
        }

        COOL_REPR_SETTER_GETTER(StringAttr, Id, id)
        COOL_REPR_SETTER_GETTER(StringAttr, Type, type)
        COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
    };

  private:
    Expr* expr;
    vector<Branch*> branches;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Case)

    Case(Expr* _expr, const vector<Branch*>& _branches)
    : expr(_expr), branches(_branches) {}

    Case* Clone() final;

    diag::TextInfo GetTextInfo() const final {
        return expr->GetTextInfo();
    }

    COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
    COOL_REPR_SETTER_GETTER(vector<Branch*>, Branches, branches)
};

//======================================================================//
//                            New Class                                 //
//======================================================================//
class New : public Expr {
  private:
    StringAttr type;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(New)

    New(const StringAttr& _type) : type(_type) {}

    New* Clone() final { return new New(type); }

    diag::TextInfo GetTextInfo() const final { return type.TextInfo(); }

    COOL_REPR_SETTER_GETTER(StringAttr, Type, type)
};

//======================================================================//
//                          Unary Class                                 //
//======================================================================//
class Unary : public Expr {
  protected:
    Expr* expr;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Unary)

    Unary(Expr* _expr) : expr(_expr) {}

    diag::TextInfo GetTextInfo() const final {
        return expr->GetTextInfo();
    }

    COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
};

//======================================================================//
//                          IsVoid Class                                //
//======================================================================//
class IsVoid : public Unary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(IsVoid)

    IsVoid(Expr* _expr) : Unary(_expr) {}

    IsVoid* Clone() final { return new IsVoid(expr->Clone()); }
};

//======================================================================//
//                          Negate Class                                //
//======================================================================//
class Negate : public Unary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(Negate)

    Negate(Expr* _expr) : Unary(_expr) {}

    Negate* Clone() final { return new Negate(expr->Clone()); }
};

//======================================================================//
//                             Not Class                                //
//======================================================================//
struct Not : public Unary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(Not)

    Not(Expr* _expr) : Unary(_expr) {}

    Not* Clone() final { return new Not(expr->Clone()); }
};

//======================================================================//
//                          Binary Class                                //
//======================================================================//
class Binary : public Expr {
  protected:
    Expr* left;
    Expr* right;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Binary)

    Binary(Expr* _left, Expr* _right) : left(_left), right(_right) {}

    diag::TextInfo GetTextInfo() const final {
        return left->GetTextInfo();
    }

    COOL_REPR_SETTER_GETTER_POINTER(Expr, Left, left)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, Right, right)
};

//======================================================================//
//                             Add Class                                //
//======================================================================//
class Add : public Binary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(Add)

    Add(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    Add* Clone() final {
        return new Add(left->Clone(), right->Clone());
    }
};

//======================================================================//
//                           Minus Class                                //
//======================================================================//
class Minus : public Binary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(Minus)

    Minus(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    Minus* Clone() final {
        return new Minus(left->Clone(), right->Clone());
    }
};

//======================================================================//
//                           Multiply Class                             //
//======================================================================//
class Multiply : public Binary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(Multiply)

    Multiply(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    Multiply* Clone() final {
        return new Multiply(left->Clone(), right->Clone());
    }
};

//======================================================================//
//                             Divide Class                             //
//======================================================================//
class Divide : public Binary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(Divide)

    Divide(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    Divide* Clone() final {
        return new Divide(left->Clone(), right->Clone());
    }
};

//======================================================================//
//                             LessThan Class                           //
//======================================================================//
class LessThan : public Binary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(LessThan)

    LessThan(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    LessThan* Clone() final {
        return new LessThan(left->Clone(), right->Clone());
    }
};

//======================================================================//
//                      LessThanOrEqual Class                           //
//======================================================================//
class LessThanOrEqual : public Binary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(LessThanOrEqual)

    LessThanOrEqual(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    LessThanOrEqual* Clone() final {
        return new LessThanOrEqual(
            left->Clone(), right->Clone());
    }
};

//======================================================================//
//                           Equal Class                                //
//======================================================================//
class Equal : public Binary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(Equal)

    Equal(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    Equal* Clone() final {
        return new Equal(left->Clone(), right->Clone());
    }
};

//======================================================================//
//                        MethodCall Class                              //
//======================================================================//
class MethodCall : public Binary {
  private:
    string type;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(MethodCall)

    MethodCall(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    MethodCall* Clone() final {
        return new MethodCall(left->Clone(), right->Clone());
    }

    COOL_REPR_SETTER_GETTER(string, Type, type)
};

//======================================================================//
//                            nteger Class                              //
//======================================================================//
class Integer : public Expr {
  private:
    IntAttr val;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Integer)

    Integer(const IntAttr& _val) : val(_val) {}

    Integer* Clone() final { return new Integer(val); }

    IntAttr Value() { return val; }

    diag::TextInfo GetTextInfo() const final { return val.TextInfo(); }

    COOL_REPR_SETTER_GETTER(IntAttr, Value, val)
};

//======================================================================//
//                            String Class                              //
//======================================================================//
class String : public Expr {
  private:
    StringAttr val;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(String)

    String(const StringAttr& _val) : val(_val) {}

    String* Clone() final { return new String(val); }

    StringAttr Value() { return val; }

    diag::TextInfo GetTextInfo() const final { return val.TextInfo(); }

    COOL_REPR_SETTER_GETTER(StringAttr, Value, val)
};

//======================================================================//
//                              True Class                              //
//======================================================================//
class True : public Expr {
  private:
    diag::TextInfo textInfo;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(True)

    True(const diag::TextInfo& _textInfo) : textInfo(_textInfo) {}

    True* Clone() { return new True(textInfo); }

    diag::TextInfo GetTextInfo() const final { return textInfo; }
};

//======================================================================//
//                            False Class                               //
//======================================================================//
class False : public Expr {
  private:
    diag::TextInfo textInfo;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(False)

    False(const diag::TextInfo& _textInfo) : textInfo(_textInfo) {}

    False* Clone() final { return new False(textInfo); }

    diag::TextInfo GetTextInfo() const final { return textInfo; }
};

//======================================================================//
//                        FuncFeature Class                             //
//======================================================================//
class FuncFeature : public Repr {
  private:
    StringAttr name;
    StringAttr type;
    Expr* expr;
    vector<Formal*> args;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(FuncFeature)

    FuncFeature(const StringAttr& _name, const StringAttr& _type,
        Expr* _expr, vector<Formal*> _args = {})
    : name(_name), type(_type), expr(_expr), args(_args) {}

    FuncFeature* Clone() final;

    diag::TextInfo GetTextInfo() const final { return name.TextInfo(); }

    COOL_REPR_SETTER_GETTER(StringAttr, Name, name)
    COOL_REPR_SETTER_GETTER(StringAttr, Type, type)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
    COOL_REPR_SETTER_GETTER(vector<Formal*>, Args, args)
};

//======================================================================//
//                        FieldFeature Class                            //
//======================================================================//
class FieldFeature : public Repr {
  private:
    StringAttr name;
    StringAttr type;
    Expr* expr;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(FieldFeature)

    FieldFeature(const StringAttr& _name,
        const StringAttr& _type, Expr* _expr)
    : name(_name), type(_type), expr(_expr) {}

    FieldFeature* Clone() final {
        return new FieldFeature(name, type, expr);
    }

    diag::TextInfo GetTextInfo() const final { return name.TextInfo(); }

    COOL_REPR_SETTER_GETTER(StringAttr, Name, name)
    COOL_REPR_SETTER_GETTER(StringAttr, Type, type)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
};

//======================================================================//
//                         Class Class                                  //
//======================================================================//
class Class : public Repr {
  private:
    StringAttr name;
    StringAttr parent;
    vector<FuncFeature*> funcs;
    vector<FieldFeature*> fields;
    unordered_map<string, FuncFeature*> funcMap;
    unordered_map<string, FieldFeature*> fieldMap;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Class)
    Class(const StringAttr&, const StringAttr&,
        vector<FuncFeature*>, vector<FieldFeature*>);

    Class* Clone() final;

    diag::TextInfo GetTextInfo() const final { return name.TextInfo(); }

    FuncFeature* GetFuncFeaturePtr(const string& name) {
        return funcMap.find(name) == funcMap.end() ?
        nullptr : funcMap.at(name);
    }

    vector<FuncFeature*>& GetFuncFeatures() { return funcs; }

    FieldFeature* GetFieldFeaturePtr(const string& name) {
        return fieldMap.find(name) == fieldMap.end() ?
        nullptr : fieldMap.at(name);
    }

    vector<FieldFeature*>& GetFieldFeatures() { return fields; }

    bool AddFuncFeature(FuncFeature* feat) {
        if (funcMap.find(feat->GetName().Value()) != funcMap.end())
            return false;
        funcs.emplace_back(feat);
        funcMap.insert({feat->GetName().Value(), feat});
        return true;
    }

    bool AddFieldFeature(FieldFeature* feat) {
        if (fieldMap.find(feat->GetName().Value()) != fieldMap.end())
            return false;
        fields.emplace_back(feat);
        fieldMap.insert({feat->GetName().Value(), feat});
        return true;
    }

    void DeleteFuncFeature(const string&);
    void DeleteFieldFeature(const string& name);

    COOL_REPR_SETTER_GETTER(StringAttr, Name, name);
    COOL_REPR_SETTER_GETTER(StringAttr, Parent, parent);
};

//======================================================================//
//                         Program Class                                //
//======================================================================//
class Program : public Repr {
  private:
    diag::TextInfo textInfo;
    vector<Class*> classVec;
    unordered_map<string, Class*> classMap;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Program)

    Program(const diag::TextInfo&, const vector<Class*>&);

    Program* Clone() final;

    diag::TextInfo GetTextInfo() const final { return textInfo; }

    Class* GetClassPtr(const string& name) {
        return classMap.find(name) != classMap.end() ?
        classMap.at(name) : nullptr;
    }

    vector<Class*> GetClasses() { return classVec; }

    bool AddClass(Class* cls) {
        if (GetClassPtr(cls->GetName().Value()))
            return false;
        classVec.emplace_back(cls);
        classMap.insert({cls->GetName().Value(), cls});
        return true;
    }

    void DeleteClass(const string&);
};

} // expr

} // cool

#endif //C_PARSER_H
