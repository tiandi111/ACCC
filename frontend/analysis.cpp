//
// Created by 田地 on 2021/6/14.
//

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <vector>

#include "analysis.h"
#include "builtin.h"
#include "pass.h"
#include "adt.h"
#include "visitor.h"
#include "typead.h"
#include "repr.h"
#include "token.h"

using namespace std;
using namespace cool;
using namespace visitor;
using namespace adt;

repr::Program* ana::InstallBuiltin::operator()(repr::Program* prog, pass::PassContext& ctx) {

    for (auto& cls : builtin::NewBuiltinClasses()) {
        if (!prog->AddClass(cls)) {
            ctx.diag.EmitError(prog->GetClassPtr(cls->GetName().Value())->GetTextInfo(),
                "built-in class '" + cls->GetName().Value() + "' cannot be redefined");
            prog->DeleteClass(cls->GetName().Value());
            assert(prog->AddClass(cls));
        }
    }

    for (auto& cls : prog->GetClasses()) {
        if (cls->GetParent().Empty() && cls->GetName().Value() != "Object")
            cls->SetParent({"Object", cls->GetParent().TextInfo()});
    }
    return prog;
}

repr::Class* ana::CheckBuiltinInheritance::operator()(repr::Class* cls, pass::PassContext& ctx) {
    using namespace builtin;
    if (IsBuiltinClass(cls->GetParent().Value()) && !IsInheritable(cls->GetParent().Value())) {
        ctx.diag.EmitError(cls->GetParent().TextInfo(),
            "cannot inherit built-in class '" + cls->GetParent().Value() + "'");
        cls->SetParent({"Object", cls->GetParent().TextInfo()}); //
    }
    return cls;
}

repr::Program* ana::BuildInheritanceTree::operator()(repr::Program* prog, pass::PassContext& ctx) {
    type::TypeAdvisor typeAdvisor(prog->GetClassPtr("Object"));

    vector<repr::Class*> stack;

    auto emitCycleError = [](vector<repr::Class*>& cycle, diag::Diagnosis& diag) {
        assert(cycle.size() > 1);
        string str = "cyclic inheritance detected: ";
        for (int i = 0; i < cycle.size() - 1; i++) {
            str += i == 0 ? "" : ", ";
            str += "'" + cycle.at(i)->GetName().Value() + "' inherits '" + cycle.at(i+1)->GetName().Value() + "'";
        }
        diag.EmitFatal(cycle.back()->GetTextInfo(), str);
    };

    auto add = [](vector<repr::Class*>& classes, type::TypeAdvisor& typeAd) {
        while (!classes.empty()) {
            typeAd.AddType(classes.back());
            classes.pop_back();
        }
    };

    for (auto cls : prog->GetClasses()) {
        unordered_set<string> visiting;
        while (cls) {
            if (typeAdvisor.Contains(cls->GetName().Value()))
                break;
            stack.emplace_back(cls);
            if (visiting.find(cls->GetName().Value()) != visiting.end()) {
                emitCycleError(stack, ctx.diag);
                return prog;
            }
            visiting.insert(cls->GetName().Value());
            cls = prog->GetClassPtr(cls->GetParent().Value());
        }
        add(stack, typeAdvisor);
    }


    ctx.Set<type::TypeAdvisor>("type_advisor", typeAdvisor);
    return prog;
}

repr::Class* ana::CheckInheritedAttributes::operator()(repr::Class* cls, pass::PassContext& ctx) {
    auto& typeAdvisor = *ctx.Get<type::TypeAdvisor>("type_advisor");

    auto check = [&](repr::Class* ancestor) {
        for (auto& field : cls->GetFieldFeatures()) {
            if (ancestor->GetFieldFeaturePtr(field->GetName().Value())) {
                ctx.diag.EmitError(field->GetTextInfo(),
                    "inherited attribute '" + field->GetName().Value() + "' cannot be redefined");
            }
        }
        return false;
    };

    typeAdvisor.BottomUpVisit(cls->GetParent().Value(), check);
    return cls;
}

repr::Class* ana::AddInheritedAttributes::operator()(repr::Class* cls, pass::PassContext& ctx) {
    using namespace repr;
    auto& typeAdvisor = *ctx.Get<type::TypeAdvisor>("type_advisor");

    vector<FieldFeature*> feats = cls->GetFieldFeatures();
    for (auto& feat : feats)
        cls->DeleteFieldFeature(feat->GetName().Value());

    stack<Class*> stack;
    Class* cur = typeAdvisor.GetTypeRepr(cls->GetParent().Value());
    while (cur) {
        stack.push(cur);
        cur = typeAdvisor.GetTypeRepr(cur->GetParent().Value());
    }

    while (!stack.empty()) {
        for (auto& field : stack.top()->GetFieldFeatures())
            cls->AddFieldFeature(field->Clone());
        stack.pop();
    }

    for(auto& feat : feats)
        cls->AddFieldFeature(feat);
    return cls;
}

repr::Class* ana::CheckInheritedMethods::operator()(repr::Class* cls, pass::PassContext& ctx) {
    auto& typeAdvisor = *ctx.Get<type::TypeAdvisor>("type_advisor");

    auto valid = [](repr::FuncFeature& a, repr::FuncFeature& b) {
        if (a.GetName().Value() != b.GetName().Value() ||
            a.GetType().Value() != b.GetType().Value() ||
            a.GetArgs().size() != b.GetArgs().size())
            return false;
        for(int i = 0; i < a.GetArgs().size(); i++) {
            if (a.GetArgs().at(i)->GetType().Value() != b.GetArgs().at(i)->GetType().Value())
                return false;
        }
        return true;
    };

    auto check = [&](repr::Class* ancestor) {
        for (auto& func : cls->GetFuncFeatures()) {

            if (ancestor->GetFuncFeaturePtr(func->GetName().Value()) &&
                !valid(*func, *(ancestor->GetFuncFeaturePtr(func->GetName().Value())))) {
                ctx.diag.EmitError(func->GetTextInfo(),
                    "invalid method overload: '" + func->GetName().Value() + "'");
            }

        }
        return false;
    };

    typeAdvisor.BottomUpVisit(cls->GetParent().Value(), check);
    return cls;
}

repr::Class* ana::AddInheritedMethods::operator()(repr::Class* cls, pass::PassContext& ctx) {
    auto& typeAdvisor = *ctx.Get<type::TypeAdvisor>("type_advisor");

    auto cur = typeAdvisor.GetTypeRepr(cls->GetParent().Value());
    while (cur) {
        for (auto& func : cur->GetFuncFeatures())
            cls->AddFuncFeature(func->Clone());
        cur = typeAdvisor.GetTypeRepr(cur->GetParent().Value());
    }

    return cls;
}

repr::Program* ana::InitSymbolTable::operator()(repr::Program* prog, pass::PassContext& ctx) {
    using namespace visitor;
    using namespace attr;

    class Visitor : public ProgramVisitor<void>, ClassVisitor<void>, FuncFeatureVisitor<void>,
        FieldFeatureVisitor<void, int>, FormalVisitor<void, int>, ExprVisitor<void> {
      public:
        ScopedTableSpecializer<SymbolTable> stable;

        Visitor() {
            stable.InitTraverse();
        }

        void Visit(repr::Program &prog) {
            NEW_SCOPE_GUARD(stable, {
                for (auto &cls : prog.GetClasses())
                    Visit(cls);
            }, nullptr)
        }

        void Visit(repr::Class* cls) {
            NEW_SCOPE_GUARD(stable, {
                for (auto &feat : cls->GetFuncFeatures()) Visit(*feat);
                for (int i = 0; i < cls->GetFieldFeatures().size(); i++)
                    Visit(*cls->GetFieldFeatures().at(i), i);
            }, cls)
        }

        void Visit(repr::FuncFeature &feat) {
            NEW_SCOPE_GUARD(stable, {
                for (int i = 0; i < feat.GetArgs().size(); i++)
                    Visit(*feat.GetArgs().at(i), i);
                ExprVisitor::Visit(*feat.GetExpr());
            }, stable.GetClass())
        }

        void Visit(repr::FieldFeature &feat, int idx) {
            stable.Current().Insert(IdAttr{IdAttr::Field, idx, feat.GetName().Value(), feat.GetType().Value()});
            if (feat.GetExpr())
                ExprVisitor::Visit(*feat.GetExpr());
        }

        void Visit(repr::Formal &form, int idx) {
            stable.Current().Insert(IdAttr{IdAttr::Arg, idx, form.GetName().Value(), form.GetType().Value()});
        }

        void Visit_(repr::LinkBuiltin& expr) {}

        void Visit_(repr::Assign& expr) {}

        void Visit_(repr::Add& expr) { VisitBinary(expr); }

        void Visit_(repr::Block& expr) {
            NEW_SCOPE_GUARD(stable, {
                for (auto &x : expr.GetExprs())
                    ExprVisitor::Visit(*x);
            }, stable.GetClass())
        }

        void Visit_(repr::Case& expr) {
            ExprVisitor::Visit(*expr.GetExpr());
            for (auto& branch : expr.GetBranches()) {
                NEW_SCOPE_GUARD(stable, {
                    stable.Current().Insert(IdAttr{IdAttr::Local, stable.Current().NextLocalIdIdx(),
                                                   branch->GetId().Value(), branch->GetType().Value()});
                    ExprVisitor::Visit(*branch->GetExpr());
                }, stable.GetClass())
            }
        }

        // this function does not take care of scope
        void VisitCall(repr::Call& expr) {
            Visit_(*expr.GetId());
            for (auto &arg : expr.GetArgs())
                ExprVisitor::Visit(*arg);
        }

        void Visit_(repr::Call& expr) {
            NEW_SCOPE_GUARD(stable, {
                 VisitCall(expr);
            }, stable.GetClass())
        }

        void Visit_(repr::Divide& expr) { VisitBinary(expr); }
        void Visit_(repr::Equal& expr) { VisitBinary(expr); }
        void Visit_(repr::False& expr) { return; }
        void Visit_(repr::ID& expr) { return; }

        void Visit_(repr::IsVoid& expr) {
            ExprVisitor::Visit(*expr.GetExpr());
        }
        void Visit_(repr::Integer& expr) { return; }

        void Visit_(repr::If& expr) {
            NEW_SCOPE_GUARD(stable,{
                ExprVisitor::Visit(*expr.GetIfExpr());
                NEW_SCOPE_GUARD(stable, ExprVisitor::Visit(*expr.GetThenExpr()), stable.GetClass());
                NEW_SCOPE_GUARD(stable, ExprVisitor::Visit(*expr.GetElseExpr()), stable.GetClass());
            }, stable.GetClass())
        }

        void Visit_(repr::LessThanOrEqual& expr) { VisitBinary(expr); }
        void Visit_(repr::LessThan& expr) { VisitBinary(expr); }

        void Visit_(repr::Let& expr) {
            auto decls = expr.GetDecls();
            for (int i = 0; i < decls.size(); i++) {
                stable.NewScope(stable.GetClass());
                auto* decl = decls.at(i);
                stable.Current().Insert(IdAttr{IdAttr::Local, stable.Current().NextLocalIdIdx(),
                                               decl->GetName().Value(), decl->GetType().Value()});
                if (decl->GetExpr())
                    ExprVisitor::Visit(*decl->GetExpr());
            }
            ExprVisitor::Visit(*expr.GetExpr());
            for (int i = 0; i < expr.GetDecls().size(); i++)
                stable.FinishScope();
        }

        void Visit_(repr::MethodCall& expr) {
            NEW_SCOPE_GUARD(stable, {
                ExprVisitor::Visit(*expr.GetLeft());
                VisitCall(*static_cast<repr::Call*>(expr.GetRight()));
            }, stable.GetClass())
        }

        void Visit_(repr::Multiply& expr) { VisitBinary(expr); }
        void Visit_(repr::Minus& expr) { VisitBinary(expr); }

        void Visit_(repr::Negate& expr) {
            ExprVisitor::Visit(*expr.GetExpr());
        }
        void Visit_(repr::New& expr) { return; }

        void Visit_(repr::Not& expr) {
            ExprVisitor::Visit(*expr.GetExpr());
        }

        void Visit_(repr::String& expr) { return; }
        void Visit_(repr::True& expr) { return; }
        void Visit_(repr::While& expr) {
            NEW_SCOPE_GUARD(stable, {
                ExprVisitor::Visit(*expr.GetWhileExpr());
                ExprVisitor::Visit(*expr.GetLoopExpr());
            }, stable.GetClass())
        }

        void VisitBinary(repr::Binary& expr) {
            ExprVisitor::Visit(*expr.GetLeft());
            ExprVisitor::Visit(*expr.GetRight());
        }
    };

    Visitor vis;
    vis.Visit(*prog);
    ctx.Set<ScopedTableSpecializer<SymbolTable>>("symbol_table", vis.stable);
    return prog;
}

repr::Program* ana::TypeChecking::operator()(repr::Program* prog, pass::PassContext& ctx) {
    using namespace builtin;
    using namespace tok;

    using TypeName = string;

    class Visitor : public ProgramVisitor<void>, ClassVisitor<void>, FuncFeatureVisitor<void>,
        FieldFeatureVisitor<void>, FormalVisitor<void>, ExprVisitor<TypeName> {
      private:
        string invalidAssignmentMsg(const TypeName& assignType, const TypeName& toType) {
            return string("cannot assign value of '" + assignType + "' to '" + toType +  "'");
        }

        string idNotFound(const TypeName& id) {
            return string("identifier '" + id + "' not found");
        }

    public:
        pass::PassContext& ctx;
        ScopedTableSpecializer<SymbolTable>& stable;
        type::TypeAdvisor& typeAdvisor;

        Visitor(pass::PassContext& _ctx, ScopedTableSpecializer<SymbolTable>& _stable, type::TypeAdvisor& _typeAdvisor)
        : ctx(_ctx), stable(_stable), typeAdvisor(_typeAdvisor) {
            stable.InitTraverse();
        }

        void Visit(repr::Program &prog) {
            ENTER_SCOPE_GUARD(stable,for (auto& cls : prog.GetClasses()) Visit(*cls);)
        }

        void Visit(repr::Class &cls) {
            ENTER_SCOPE_GUARD(stable, {
                for (auto& feat : cls.GetFieldFeatures())
                    Visit(*feat);
                for (auto& feat : cls.GetFuncFeatures())
                    Visit(*feat);
            })
        }

        void Visit(repr::FieldFeature &feat) {
            if (!feat.GetExpr()) return;
            TypeName exprType = ExprVisitor<TypeName>::Visit(*feat.GetExpr());
            if (!typeAdvisor.Conforms(exprType, feat.GetType().Value(), stable.GetClass()->GetName().Value()))
                ctx.diag.EmitError(feat.GetTextInfo(), invalidAssignmentMsg(exprType, feat.GetType().Value()));
        }

        void Visit(repr::FuncFeature &feat) {
            ENTER_SCOPE_GUARD(stable, {
                TypeName exprType = ExprVisitor<TypeName>::Visit(*feat.GetExpr());
                if (!typeAdvisor.Conforms(exprType, feat.GetType().Value(), stable.GetClass()->GetName().Value()))
                    ctx.diag.EmitError(feat.GetTextInfo(), invalidAssignmentMsg(exprType, feat.GetType().Value()));
            })
        }

        void Visit(repr::Formal &form) {}

        TypeName Visit_(repr::LinkBuiltin& expr) { return expr.GetType(); }

        TypeName Visit_(repr::Assign& expr) {
            auto exprType = ExprVisitor<TypeName>::Visit(*expr.GetExpr());
            auto idAttr = stable.GetIdAttr(expr.GetId()->GetName().Value());
            if (!idAttr)
                ctx.diag.EmitError(expr.GetId()->GetTextInfo(), idNotFound(expr.GetId()->GetName().Value()));
            else if (!typeAdvisor.Conforms(exprType, idAttr->type, stable.GetClass()->GetName().Value()))
                ctx.diag.EmitError(expr.GetExpr()->GetTextInfo(), invalidAssignmentMsg(exprType, idAttr->type));
            return exprType;
        }

        TypeName Visit_(repr::Add& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.GetLeft()) != "Int")
                ctx.diag.EmitError(expr.GetLeft()->GetTextInfo(), "operand for '+' must be 'Int'");
            if (ExprVisitor<TypeName>::Visit(*expr.GetRight()) != "Int")
                ctx.diag.EmitError(expr.GetRight()->GetTextInfo(), "operand for '+' must be 'Int'");
            return "Int";
        }

        TypeName Visit_(repr::Block& expr) {
            TypeName rType;
            ENTER_SCOPE_GUARD(stable, {
                for (int i = 0; i < expr.GetExprs().size() - 1; i++)
                    ExprVisitor<TypeName>::Visit(*expr.GetExprs().at(i));
                rType = ExprVisitor<TypeName>::Visit(*expr.GetExprs().back());
            })
            return rType;;
        }

        TypeName Visit_(repr::Case& expr) {
            unordered_set<TypeName> typeSet;
            vector<TypeName> types;
            for (auto& branch : expr.GetBranches()) {
                ENTER_SCOPE_GUARD(stable, {
                    auto type = ExprVisitor<TypeName>::Visit(*branch->GetExpr());
                    if (typeSet.find(type) != typeSet.end())
                        ctx.diag.EmitError(branch->GetType().TextInfo(),
                            "duplicate type '" + type + "' in case expression");
                    typeSet.insert(type);
                    types.emplace_back(type);
                })
            }
            return typeAdvisor.LeastCommonAncestor(types);
        }

        TypeName Visit_(repr::Call& expr) {
            TypeName rType;
            ENTER_SCOPE_GUARD(stable,
                auto funcPtr = CheckCall(stable.GetClass()->GetName().Value(), expr);
                if (funcPtr) {
                    expr.SetLink(funcPtr);
                    rType = funcPtr->GetType().Value();
                })
            return rType;
        }

        TypeName Visit_(repr::MethodCall& expr) {
            TypeName rType;
            ENTER_SCOPE_GUARD(stable,
                auto callExpr = static_cast<repr::Call*>(expr.GetRight());
                expr.SetType(ExprVisitor<TypeName>::Visit(*expr.GetLeft()));
                auto funcPtr = CheckCall(expr.GetType(), *callExpr);
                if (funcPtr) {
                    callExpr->SetLink(funcPtr);
                    rType = funcPtr->GetType().Value();
                })
            return rType;
        }

        repr::FuncFeature* CheckCall(TypeName type, repr::Call& expr) {
            auto cls = typeAdvisor.GetTypeRepr(type);
            if (!cls) {
                ctx.diag.EmitError(expr.GetTextInfo(), "caller type '" + type + "' not found");
                return nullptr;
            }
            auto funcPtr = cls->GetFuncFeaturePtr(expr.GetId()->GetName().Value());
            if (!funcPtr) {
                ctx.diag.EmitError(expr.GetTextInfo(), "method '" + expr.GetId()->GetName().Value() + "' not found");
                return nullptr;
            }
            if (expr.GetArgs().size() != funcPtr->GetArgs().size()) {
                ctx.diag.EmitError(expr.GetTextInfo(), "expected " + to_string(funcPtr->GetArgs().size()) +
                " arguments, got " + to_string(expr.GetArgs().size()));
                return funcPtr;
            }
            for (int i = 0; i < expr.GetArgs().size(); i++) {
                auto& arg = expr.GetArgs().at(i);
                TypeName got = ExprVisitor<TypeName>::Visit(*arg);
                TypeName expected = funcPtr->GetArgs().at(i)->GetType().Value();
                if (!typeAdvisor.Conforms(got, expected, got)) {
                    ctx.diag.EmitError(expr.GetTextInfo(), "invalid argument '" +
                    funcPtr->GetArgs().at(i)->GetName().Value() + "': expected '" + expected +
                    "', got '" + got + "'");
                    return funcPtr;
                }
            }
            return funcPtr;
        }

        TypeName Visit_(repr::Divide& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.GetLeft()) != "Int")
                ctx.diag.EmitError(expr.GetLeft()->GetTextInfo(), "operand for '/' must be 'Int'");
            if (ExprVisitor<TypeName>::Visit(*expr.GetRight()) != "Int")
                ctx.diag.EmitError(expr.GetRight()->GetTextInfo(), "operand for '/' must be 'Int'");
            return "Int";
        }

        TypeName Visit_(repr::Equal& expr) {
            auto leftType = ExprVisitor<TypeName>::Visit(*expr.GetLeft());
            auto rightType = ExprVisitor<TypeName>::Visit(*expr.GetRight());
            if (((leftType == "Int" || leftType == "String" || leftType == "Bool") ||
                (rightType == "Int" || rightType == "String" || rightType == "Bool")) &&
                (leftType != rightType))
                ctx.diag.EmitError(expr.GetRight()->GetTextInfo(), "'Int', 'String', 'Bool' can only be compared with the same type");
            return "Bool";
        }

        TypeName Visit_(repr::False& expr) { return "Bool"; }

        TypeName Visit_(repr::ID& expr) {
            auto idAttr = stable.GetIdAttr(expr.GetName().Value());
            if (!idAttr) {
                ctx.diag.EmitError(expr.GetTextInfo(), idNotFound(expr.GetName().Value()));
                return "";
            }
            return idAttr->type;
        }

        TypeName Visit_(repr::IsVoid& expr) {
            ExprVisitor<TypeName>::Visit(*expr.GetExpr());
            return "Bool";
        }

        TypeName Visit_(repr::Integer& expr) { return "Int"; }

        TypeName Visit_(repr::If& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.GetIfExpr()) != "Bool")
                ctx.diag.EmitError(expr.GetIfExpr()->GetTextInfo(), "predicate in if statement must be 'Bool'");
            return typeAdvisor.LeastCommonAncestor(
                ExprVisitor<TypeName>::Visit(*expr.GetThenExpr()),
                ExprVisitor<TypeName>::Visit(*expr.GetElseExpr()));
        }

        TypeName Visit_(repr::LessThanOrEqual& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.GetLeft()) != "Int")
                ctx.diag.EmitError(expr.GetLeft()->GetTextInfo(), "operand for '<=' must be 'Int'");
            if (ExprVisitor<TypeName>::Visit(*expr.GetRight()) != "Int")
                ctx.diag.EmitError(expr.GetRight()->GetTextInfo(), "operand for '<=' must be 'Int'");
            return "Bool";
        }

        TypeName Visit_(repr::LessThan& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.GetLeft()) != "Int")
                ctx.diag.EmitError(expr.GetLeft()->GetTextInfo(), "operand for '<' must be 'Int'");
            if (ExprVisitor<TypeName>::Visit(*expr.GetRight()) != "Int")
                ctx.diag.EmitError(expr.GetRight()->GetTextInfo(), "operand for '<' must be 'Int'");
            return "Bool";
        }

        TypeName Visit_(repr::Let& expr) {
            TypeName rType;
            auto decls = expr.GetDecls();
            for (int i = 0; i < decls.size(); i++) {
                stable.EnterScope();
                auto* decl = decls.at(i);
                if (decl->GetExpr()) {
                    auto exprType = ExprVisitor<TypeName>::Visit(*decl->GetExpr());
                    if (!typeAdvisor.Conforms(exprType, decl->GetType().Value(), stable.GetClass()->GetName().Value()))
                        ctx.diag.EmitError(decl->GetExpr()->GetTextInfo(),
                            invalidAssignmentMsg(exprType, decl->GetType().Value()));
                }
            }
            rType = ExprVisitor<TypeName>::Visit(*expr.GetExpr());
            for (int i = 0; i < expr.GetDecls().size(); i++) stable.LeaveScope();
            return rType;
        }

        TypeName Visit_(repr::Multiply& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.GetLeft()) != "Int")
                ctx.diag.EmitError(expr.GetLeft()->GetTextInfo(), "operand for '*' must be 'Int'");
            if (ExprVisitor<TypeName>::Visit(*expr.GetRight()) != "Int")
                ctx.diag.EmitError(expr.GetRight()->GetTextInfo(), "operand for '*' must be 'Int'");
            return "Int";
        }

        TypeName Visit_(repr::Minus& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.GetLeft()) != "Int")
                ctx.diag.EmitError(expr.GetLeft()->GetTextInfo(), "operand for '-' must be 'Int'");
            if (ExprVisitor<TypeName>::Visit(*expr.GetRight()) != "Int")
                ctx.diag.EmitError(expr.GetRight()->GetTextInfo(), "operand for '-' must be 'Int'");
            return "Int";
        }

        TypeName Visit_(repr::Negate& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.GetExpr()) != "Int")
                ctx.diag.EmitError(expr.GetTextInfo(), "operand for '~' must be 'Int'");
            return "Int";
        }

        TypeName Visit_(repr::New& expr) {
            return expr.GetType().Value();
        }

        TypeName Visit_(repr::Not& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.GetExpr()) != "Bool")
                ctx.diag.EmitError(expr.GetTextInfo(), "operand for 'not' must be 'Int'");
            return "Bool";
        }

        TypeName Visit_(repr::String& expr) { return "String"; }

        TypeName Visit_(repr::True& expr) { return "Bool"; }

        TypeName Visit_(repr::While& expr) {
            ENTER_SCOPE_GUARD(stable, {
                if (ExprVisitor<TypeName>::Visit(*expr.GetWhileExpr()) != "Bool")
                    ctx.diag.EmitError(expr.GetTextInfo(), "predicate in while expression must be 'Bool'");
                ExprVisitor<TypeName>::Visit(*expr.GetLoopExpr());
            })
            return "Object";
        }
    };

    auto stable = ctx.Get<ScopedTableSpecializer<SymbolTable>>("symbol_table");
    auto typeAdvisor = ctx.Get<type::TypeAdvisor>("type_advisor");

    Visitor vis(ctx, *stable, *typeAdvisor);
    vis.Visit(*prog);
    return prog;
}

repr::Program* ana::EliminateSelfType::operator()(repr::Program* prog, pass::PassContext& ctx) {

    class Visitor : public ProgramVisitor<void>, ClassVisitor<void>, FuncFeatureVisitor<void>,
        FieldFeatureVisitor<void>, FormalVisitor<void>, ExprVisitor<void> {
      private:
        ScopedTableSpecializer<SymbolTable>& stable;

      public:
        Visitor(ScopedTableSpecializer<SymbolTable>& _stable) : stable(_stable) {
            stable.InitTraverse();
        }

        void Visit(repr::Program &prog) {
            ENTER_SCOPE_GUARD(stable,for (auto& cls : prog.GetClasses()) Visit(*cls);)
        }

        void Visit(repr::Class &cls) {
            ENTER_SCOPE_GUARD(stable, {
                for (auto& feat : cls.GetFieldFeatures())
                    Visit(*feat);
                for (auto& feat : cls.GetFuncFeatures())
                    Visit(*feat);
            })
        }

        void Visit(repr::FieldFeature &feat) {
            if (feat.GetType().Value() == "SELF_TYPE")
                feat.SetType({stable.GetClass()->GetName().Value(), feat.GetType().TextInfo()});
            if (feat.GetExpr())
                Visit(*feat.GetExpr());
        }

        void Visit(repr::FuncFeature &feat) {
            ENTER_SCOPE_GUARD(stable, {
                if (feat.GetType().Value() == "SELF_TYPE")
                    feat.SetType({stable.GetClass()->GetName().Value(), feat.GetType().TextInfo()});
                Visit(*feat.GetExpr());
            })
        }

        void Visit(repr::Formal &form) {}

        void Visit(repr::Expr& expr) {
            ExprVisitor<void>::Visit(expr);
        }

        void Visit_(repr::LinkBuiltin& expr) {
            if (expr.GetType() == "SELF_TYPE")
                expr.SetType(stable.GetClass()->GetName().Value());
        }

        void Visit_(repr::Assign& expr) { Visit(*expr.GetExpr()); }

        void Visit_(repr::Add& expr) {
            Visit(*expr.GetLeft());
            Visit(*expr.GetLeft());
        }

        void Visit_(repr::Block& expr) {
            ENTER_SCOPE_GUARD(stable, {
                for (int i = 0; i < expr.GetExprs().size() - 1; i++)
                    Visit(*expr.GetExprs().at(i));
            })
        }

        void Visit_(repr::Case& expr) {
            Visit(*expr.GetExpr());
            for (auto& branch : expr.GetBranches()) {
                ENTER_SCOPE_GUARD(stable, Visit(*branch->GetExpr());)
            }
        }

        void VisitCall(repr::Call& expr) {
            for (auto& arg : expr.GetArgs()) Visit(*arg);
        }

        void Visit_(repr::Call& expr) {
            ENTER_SCOPE_GUARD(stable,VisitCall(expr))
        }

        void Visit_(repr::MethodCall& expr) {
            ENTER_SCOPE_GUARD(stable, {
                Visit(*expr.GetLeft());
                VisitCall(*static_cast<repr::Call*>(expr.GetRight()));
            })
        }

        void Visit_(repr::Divide& expr) {
            Visit(*expr.GetLeft());
            Visit(*expr.GetRight());
        }

        void Visit_(repr::Equal& expr) {
            Visit(*expr.GetLeft());
            Visit(*expr.GetRight());
        }

        void Visit_(repr::False& expr) {}

        void Visit_(repr::ID& expr) {}

        void Visit_(repr::IsVoid& expr) { Visit(*expr.GetExpr()); }

        void Visit_(repr::Integer& expr) {}

        void Visit_(repr::If& expr) {
            Visit(*expr.GetIfExpr());
            Visit(*expr.GetThenExpr());
            Visit(*expr.GetElseExpr());
        }

        void Visit_(repr::LessThanOrEqual& expr) {
            Visit(*expr.GetLeft());
            Visit(*expr.GetRight());
        }

        void Visit_(repr::LessThan& expr) {
            Visit(*expr.GetLeft());
            Visit(*expr.GetRight());
        }

        void Visit_(repr::Let& expr) {
            auto decls = expr.GetDecls();
            for (int i = 0; i < decls.size(); i++) {
                stable.EnterScope();
                if (decls.at(i)->GetExpr())
                    Visit(*decls.at(i)->GetExpr());
            }
            Visit(*expr.GetExpr());
            for (int i = 0; i < decls.size(); i++)
                stable.LeaveScope();
        }

        void Visit_(repr::Multiply& expr) {
            Visit(*expr.GetLeft());
            Visit(*expr.GetRight());
        }

        void Visit_(repr::Minus& expr) {
            Visit(*expr.GetLeft());
            Visit(*expr.GetRight());
        }

        void Visit_(repr::Negate& expr) {
            Visit(*expr.GetExpr());
        }

        void Visit_(repr::New& expr) {
            if (expr.GetType().Value() == "SELF_TYPE")
                expr.SetType({stable.GetClass()->GetName().Value(), expr.GetType().TextInfo()});
        }

        void Visit_(repr::Not& expr) { Visit(*expr.GetExpr()); }

        void Visit_(repr::String& expr) {}

        void Visit_(repr::True& expr) {}

        void Visit_(repr::While& expr) {
            ENTER_SCOPE_GUARD(stable, {
                Visit(*expr.GetWhileExpr());
                Visit(*expr.GetLoopExpr());
            })
        }
    };

    auto stable = ctx.Get<ScopedTableSpecializer<SymbolTable>>("symbol_table");
    Visitor vis(*stable);
    vis.Visit(*prog);
    return prog;
}