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

// note: provide copy constructor when there're dynamic memory allocations,
//  otherwise, the default copy constructor is enough

// note: method defined in class definition is "inlined" as default

// todo: provide move ctor
// todo: provide deep copier

// todo: is shared_ptr necessary?
//  problem: virtual functions with return type as shared_ptr<Base> cannot be
//      overriden with shared_ptr<Derived>

class Attr {
  private:
    diag::TextInfo textInfo;

  public:
    Attr(int _line = -1, int _pos = -1, int _fileno = -1) : textInfo({_line, _pos, _fileno}) {}

    Attr(const diag::TextInfo& _textInfo) : textInfo(_textInfo) {}

    Attr(const tok::Token& token) : textInfo(token.textInfo) {}

    diag::TextInfo TextInfo() const { return textInfo; }
};

class StringAttr : public Attr {
  private:
    string val;

  public:
    StringAttr() = default;

    StringAttr(const string& _val, int _line = -1, int _pos = -1, int _fileno = -1)
    : val(_val), Attr(_line, _pos, _fileno) {}

    StringAttr(const tok::Token& token) : val(token.str), Attr(token.textInfo) {}

    StringAttr(const string& _val, const diag::TextInfo& _textInfo)
    : val(_val), Attr(_textInfo) {}

    string Value() { return val; }

    void SetValue(const string& _val) { val = _val; }

    inline bool Empty() const { return val.empty(); }
};

class IntAttr : public Attr {
  private:
    int val;

  public:
    IntAttr() = default;

    IntAttr(int _val, int _line = -1, int _pos = -1, int _fileno = -1)
    : val(_val), Attr(_line, _pos, _fileno) {}

    IntAttr(const tok::Token& token) : Attr(token.textInfo) {
        if (token.type != tok::Token::Integer) throw runtime_error("");
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

// note:
//  Templates are all about the compiler generating code at compile-time.
//  Virtual functions are all about the run-time system figuring out which function to call at run-time.

// note: It is necessary to differentiate owning pointer and non-owning pointer.
// Owning pointer is a pointer that needs to be deleted by its owner.
// For owning pointer, it is better to use smart pointer to manage its life cycle.
// For non-owning pointer, raw pointer is okay.

// todo: attach original token with repr
class Repr {
  public:
    virtual ~Repr() = default;
    virtual diag::TextInfo GetTextInfo() const = 0;
    virtual Repr* Clone() = 0;
};

class Expr : public Repr {
  public:
    virtual ~Expr() = default;
    virtual Expr* Clone() = 0;
};

class Formal : public Repr {
  private:
    StringAttr name;
    StringAttr type;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Formal)

    Formal(const StringAttr& _name, const StringAttr& _type)
    : name(_name), type(_type) {}

    Formal* Clone() { return new Formal(name, type); }

    inline diag::TextInfo GetTextInfo() const final { return name.TextInfo(); }

    COOL_REPR_SETTER_GETTER(StringAttr, Name, name)
    COOL_REPR_SETTER_GETTER(StringAttr, Type, type)
};

class LinkBuiltin : public Expr {
  private:
    string name;
    string type;
    vector<string> params;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(LinkBuiltin)

    LinkBuiltin(const string& _name, const string& _type, vector<string> _params)
    : name(_name), type(_type), params(move(_params)) {}

    LinkBuiltin* Clone() {
        return new LinkBuiltin(name, type, params);
    }

    inline diag::TextInfo GetTextInfo() const final {
        return diag::TextInfo{};
    }

    COOL_REPR_SETTER_GETTER(string, Name, name)
    COOL_REPR_SETTER_GETTER(string, Type, type)
    COOL_REPR_SETTER_GETTER(vector<string>, Params, params)
};

class ID : public Expr {
  private:
    StringAttr name;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(ID)

    ID(const StringAttr& _name) : name(_name) {}

    ID* Clone() {
        return new ID(name);
    }

    inline diag::TextInfo GetTextInfo() const final {
        return name.TextInfo();
    }

    COOL_REPR_SETTER_GETTER(StringAttr, Name, name)
};

class Assign : public Expr {
  private:
    ID* id;
    Expr* expr;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Assign)

    Assign(ID* _id, Expr* _expr) : id(_id), expr(_expr) {}

    Assign* Clone() { return new Assign(id->Clone(), expr->Clone()); }

    inline diag::TextInfo GetTextInfo() const final { return id->GetTextInfo(); }

    COOL_REPR_SETTER_GETTER_POINTER(ID, Id, id)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
};

class FuncFeature;

class Call : public Expr {
  private:
    ID* id;
    vector<Expr*> args;
    FuncFeature* link;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Call)

    Call(ID* _id, vector<Expr*> _args, FuncFeature* _link) : id(_id), args(_args), link(_link) {}

    Call* Clone() { return new Call(id, args, link);  }

    inline diag::TextInfo GetTextInfo() const final { return id->GetTextInfo(); }

    COOL_REPR_SETTER_GETTER_POINTER(ID, Id, id)
    COOL_REPR_SETTER_GETTER(vector<Expr*>, Args, args)
    COOL_REPR_SETTER_GETTER_POINTER(FuncFeature, Link, link)
};

class If : public Expr {
  private:
    Expr* ifExpr;
    Expr* thenExpr;
    Expr* elseExpr;

public:
    COOL_REPR_BASE_CONSTRUCTOR(If)

    If(Expr* _ifExpr, Expr* _thenExpr, Expr* _elseExpr)
    : ifExpr(_ifExpr), thenExpr(_thenExpr), elseExpr(_elseExpr) {}

    If* Clone() { return new If(ifExpr->Clone(), thenExpr->Clone(), elseExpr->Clone()); }

    inline diag::TextInfo GetTextInfo() const final { return ifExpr->GetTextInfo(); }

    COOL_REPR_SETTER_GETTER_POINTER(Expr, IfExpr, ifExpr)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, ThenExpr, thenExpr)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, ElseExpr, elseExpr)
};

class Block : public Expr {
  private:
    diag::TextInfo textInfo;
    vector<Expr*> exprs;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Block)

    Block(diag::TextInfo _textInfo, vector<Expr*> _exprs = {})
    : textInfo(_textInfo), exprs(_exprs) {}

    Block* Clone() { return new Block(textInfo, exprs); }

    inline diag::TextInfo GetTextInfo() const final { return textInfo; }

    COOL_REPR_SETTER_GETTER(vector<Expr*>, Exprs, exprs)
};

class While : public Expr {
  private:
    Expr* whileExpr;
    Expr* loopExpr;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(While)

    While(Expr* _whileExpr, Expr* _loopExpr)
    : whileExpr(_whileExpr), loopExpr(_loopExpr) {}

    While* Clone() { new While(whileExpr->Clone(), loopExpr->Clone()); }

    inline diag::TextInfo GetTextInfo() const final { return whileExpr->GetTextInfo(); }

    COOL_REPR_SETTER_GETTER_POINTER(Expr, WhileExpr, whileExpr)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, LoopExpr, loopExpr)
};

class Let : public Expr {
  public:
    class Decl : public Repr {
      private:
        StringAttr name;
        StringAttr type;
        Expr* expr;

      public:
        COOL_REPR_BASE_CONSTRUCTOR(Decl)

        Decl(const StringAttr& _name, const StringAttr& _type, Expr* _expr)
            : name(_name), type(_type), expr(_expr) {}

        Decl* Clone() { return new Decl(name, type, expr->Clone()); }

        inline diag::TextInfo GetTextInfo() const final { return name.TextInfo(); }

        COOL_REPR_SETTER_GETTER(StringAttr, Name, name)
        COOL_REPR_SETTER_GETTER(StringAttr, Type, type)
        COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
    };

  private:
    vector<Let::Decl*> decls;
    Expr* expr;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Let)

    Let(vector<Let::Decl*> _decls, Expr* _expr) : decls(_decls), expr(_expr) {}

    Let* Clone() { return new Let(decls, expr->Clone()); }

    inline diag::TextInfo GetTextInfo() const final { return expr->GetTextInfo(); }

    COOL_REPR_SETTER_GETTER(vector<Let::Decl*>, Decls, decls)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
};

class Case : public Expr {
  public:
    class Branch : Repr {
      private:
        StringAttr id;
        StringAttr type;
        Expr* expr;

      public:
        COOL_REPR_BASE_CONSTRUCTOR(Branch)

        Branch(const StringAttr& _id, const StringAttr& _type, Expr* _expr)
        : id(_id), type(_type), expr(_expr) {}

        Branch* Clone() { return new Branch(id, type, expr->Clone()); }

        inline diag::TextInfo GetTextInfo() const final { return expr->GetTextInfo(); }

        COOL_REPR_SETTER_GETTER(StringAttr, Id, id)
        COOL_REPR_SETTER_GETTER(StringAttr, Type, type)
        COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
    };

  private:
    Expr* expr;
    vector<Branch*> branches;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Case)

    Case(Expr* _expr, vector<Branch*> _branches) : expr(_expr), branches(_branches) {}

    Case* Clone() {
        vector<Branch*> _branches(branches.size());
        for (int i = 0; i < branches.size(); i++)
            _branches[i] = branches.at(i)->Clone();
        return new Case(expr->Clone(), _branches);
    }

    inline diag::TextInfo GetTextInfo() const final { return expr->GetTextInfo(); }

    COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
    COOL_REPR_SETTER_GETTER(vector<Branch*>, Branches, branches)
};

class New : public Expr {
  private:
    StringAttr type;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(New)

    New(const StringAttr& _type) : type(_type) {}

    New* Clone() { return new New(type); }

    inline diag::TextInfo GetTextInfo() const final { return type.TextInfo(); }

    COOL_REPR_SETTER_GETTER(StringAttr, Type, type)
};

class Unary : public Expr {
  protected:
    Expr* expr;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Unary)

    Unary(Expr* _expr) : expr(_expr) {}

    Unary* Clone() { return new Unary(expr->Clone()); }

    inline diag::TextInfo GetTextInfo() const final { return expr->GetTextInfo(); }

    COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
};

class IsVoid : public Unary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(IsVoid)

    IsVoid(Expr* _expr) : Unary(_expr) {}

    IsVoid* Clone() { return new IsVoid(expr->Clone()); }
};

class Negate : public Unary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(Negate)

    Negate(Expr* _expr) : Unary(_expr) {}

    Negate* Clone() { return new Negate(expr->Clone()); }
};

struct Not : public Unary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(Not)

    Not(Expr* _expr) : Unary(_expr) {}

    Not* Clone() { return new Not(expr->Clone()); }
};

class Binary : public Expr {
  protected:
    Expr* left;
    Expr* right;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Binary)

    Binary(Expr* _left, Expr* _right) : left(_left), right(_right) {}

    Binary* Clone() { return new Binary(left->Clone(), right->Clone()); }

    inline diag::TextInfo GetTextInfo() const final { return left->GetTextInfo(); }

    COOL_REPR_SETTER_GETTER_POINTER(Expr, Left, left)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, Right, right)
};

class Add : public Binary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(Add)

    Add(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    Add* Clone() { return new Add(left->Clone(), right->Clone()); }
};

class Minus : public Binary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(Minus)

    Minus(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    Minus* Clone() { return new Minus(left->Clone(), right->Clone()); }
};

class Multiply : public Binary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(Multiply)

    Multiply(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    Multiply* Clone() { return new Multiply(left->Clone(), right->Clone()); }
};

class Divide : public Binary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(Divide)

    Divide(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    Divide* Clone() { return new Divide(left->Clone(), right->Clone()); }
};

class LessThan : public Binary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(LessThan)

    LessThan(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    LessThan* Clone() { return new LessThan(left->Clone(), right->Clone()); }
};

class LessThanOrEqual : public Binary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(LessThanOrEqual)

    LessThanOrEqual(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    LessThanOrEqual* Clone() { return new LessThanOrEqual(left->Clone(), right->Clone()); }
};

class Equal : public Binary {
  public:
    COOL_REPR_BASE_CONSTRUCTOR(Equal)

    Equal(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    Equal* Clone() { return new Equal(left->Clone(), right->Clone()); }
};

class MethodCall : public Binary {
  private:
    string type;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(MethodCall)

    MethodCall(Expr* _left, Expr* _right) : Binary(_left, _right) {}

    MethodCall* Clone() { return new MethodCall(left->Clone(), right->Clone()); }

    COOL_REPR_SETTER_GETTER(string, Type, type)
};

class Integer : public Expr {
  private:
    IntAttr val;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Integer)

    Integer(const IntAttr& _val) : val(_val) {}

    Integer* Clone() { return new Integer(val); }

    IntAttr Value() { return val; }

    inline diag::TextInfo GetTextInfo() const final { return val.TextInfo(); }

    COOL_REPR_SETTER_GETTER(IntAttr, Value, val)
};

class String : public Expr {
  private:
    StringAttr val;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(String)

    String(const StringAttr& _val) : val(_val) {}

    String* Clone() { return new String(val); }

    StringAttr Value() { return val; }

    inline diag::TextInfo GetTextInfo() const final { return val.TextInfo(); }

    COOL_REPR_SETTER_GETTER(StringAttr, Value, val)
};

class True : public Expr {
  private:
    diag::TextInfo textInfo;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(True)

    True(const diag::TextInfo& _textInfo) : textInfo(_textInfo) {}

    True* Clone() { return new True(textInfo); }

    inline diag::TextInfo GetTextInfo() const final { return textInfo; }
};

class False : public Expr {
  private:
    diag::TextInfo textInfo;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(False)

    False(const diag::TextInfo& _textInfo) : textInfo(_textInfo) {}

    False* Clone() { return new False(textInfo); }

    inline diag::TextInfo GetTextInfo() const final { return textInfo; }
};

class FuncFeature : Repr {
  private:
    StringAttr name;
    StringAttr type;
    Expr* expr;
    vector<Formal*> args;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(FuncFeature)

    FuncFeature(const StringAttr& _name, const StringAttr& _type, Expr* _expr, vector<Formal*> _args = {})
    : name(_name), type(_type), expr(_expr), args(_args) {}

    FuncFeature* Clone() {
        vector<Formal*> _args;
        for (auto& arg : args)
            _args.emplace_back(arg->Clone());
        return new FuncFeature(name, type, expr->Clone(), _args);
    }

    inline diag::TextInfo GetTextInfo() const final { return name.TextInfo(); }

    COOL_REPR_SETTER_GETTER(StringAttr, Name, name)
    COOL_REPR_SETTER_GETTER(StringAttr, Type, type)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
    COOL_REPR_SETTER_GETTER(vector<Formal*>, Args, args)
};

class FieldFeature : Repr {
  private:
    StringAttr name;
    StringAttr type;
    Expr* expr;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(FieldFeature)

    FieldFeature(const StringAttr& _name, const StringAttr& _type, Expr* _expr)
    : name(_name), type(_type), expr(_expr) {}

    FieldFeature* Clone() { return new FieldFeature(name, type, expr); }

    inline diag::TextInfo GetTextInfo() const final { return name.TextInfo(); }

    COOL_REPR_SETTER_GETTER(StringAttr, Name, name)
    COOL_REPR_SETTER_GETTER(StringAttr, Type, type)
    COOL_REPR_SETTER_GETTER_POINTER(Expr, Expr, expr)
};

class Class : Repr {
  private:
    StringAttr name;
    StringAttr parent;
    vector<FuncFeature*> funcs;
    vector<FieldFeature*> fields;
    unordered_map<string, FuncFeature*> funcMap;
    unordered_map<string, FieldFeature*> fieldMap;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Class)

    Class(const StringAttr& _name, const StringAttr& _parent, vector<FuncFeature*> _funcs = {},
        vector<FieldFeature*> _fields = {})
        : name(_name), parent(_parent), funcs(move(_funcs)), fields(move(_fields)) {
        for (auto& func : funcs) funcMap.insert({func->GetName().Value(), func});
        for (auto& field : fields) fieldMap.insert({field->GetName().Value(), field});
    }

    Class* Clone() {
        vector<FuncFeature*> _funcs(funcs.size());
        for(int i = 0; i < funcs.size(); i++)
            _funcs[i] = funcs.at(i)->Clone();
        vector<FieldFeature*> _fields(fields.size());
        for(int i = 0; i < fields.size(); i++)
            _fields[i] = fields.at(i)->Clone();

        return new Class(name, parent, _funcs, _fields);
    }

    inline diag::TextInfo GetTextInfo() const final { return name.TextInfo(); }

    inline FuncFeature* GetFuncFeaturePtr(const string& name) {
        return funcMap.find(name) == funcMap.end() ? nullptr : funcMap.at(name);
    }

    inline vector<FuncFeature*>& GetFuncFeatures() { return funcs; }

    inline FieldFeature* GetFieldFeaturePtr(const string& name) {
        return fieldMap.find(name) == fieldMap.end() ? nullptr : fieldMap.at(name);
    }

    inline vector<FieldFeature*>& GetFieldFeatures() { return fields; }

    bool AddFuncFeature(FuncFeature* feat) {
        if (funcMap.find(feat->GetName().Value()) != funcMap.end()) return false;
        funcs.emplace_back(feat);
        funcMap.insert({feat->GetName().Value(), feat});
        return true;
    }

    bool AddFieldFeature(FieldFeature* feat) {
        if (fieldMap.find(feat->GetName().Value()) != fieldMap.end()) return false;
        fields.emplace_back(feat);
        fieldMap.insert({feat->GetName().Value(), feat});
        return true;
    }

    void DeleteFuncFeature(const string& name) {
        if (!GetFuncFeaturePtr(name)) return;
        funcMap.erase(name);
        for (auto it = funcs.begin(); it != funcs.end(); it++) {
            if ((*it)->GetName().Value() == name) {
                funcs.erase(it);
                return;
            }
        }
    }

    void DeleteFieldFeature(const string& name) {
        if (!GetFieldFeaturePtr(name)) return;
        fieldMap.erase(name);
        for (auto it = fields.begin(); it != fields.end(); it++) {
            if ((*it)->GetName().Value() == name) {
                fields.erase(it);
                return;
            }
        }
    }

    COOL_REPR_SETTER_GETTER(StringAttr, Name, name);
    COOL_REPR_SETTER_GETTER(StringAttr, Parent, parent);
};

class Program : public Repr {
  private:
    diag::TextInfo textInfo;
    vector<Class*> classVec;
    unordered_map<string, Class*> classMap;

  public:
    COOL_REPR_BASE_CONSTRUCTOR(Program)

    Program(const diag::TextInfo& _textInfo, vector<Class*> _classVec = {})
    : textInfo(_textInfo), classVec(_classVec) {
        for (auto& cls : classVec)
            classMap.insert({cls->GetName().Value(), cls});
    }

    Program* Clone() {
        vector<Class*> _classVec(classVec.size());
        for (int i = 0; i < classVec.size(); i++)
            _classVec[i] = classVec.at(i)->Clone();
    }

    inline diag::TextInfo GetTextInfo() const final { return textInfo; }

    inline Class* GetClassPtr(const string& name) {
        return classMap.find(name) != classMap.end() ? classMap.at(name) : nullptr;
    }

    inline vector<Class*> GetClasses() { return classVec; }

    bool AddClass(Class* cls) {
        if (GetClassPtr(cls->GetName().Value()))
            return false;
        classVec.emplace_back(cls);
        classMap.insert({cls->GetName().Value(), cls});
        return true;
    }

    void DeleteClass(const string& name) {
        if (!GetClassPtr(name)) return;
        classMap.erase(name);
        auto it = classVec.begin();
        for (; it != classVec.end() && (*it)->GetName().Value() != name; it++) {}
        assert(it != classVec.end());
        classVec.erase(it);
    }
};

} // expr

} // cool

#endif //C_PARSER_H
