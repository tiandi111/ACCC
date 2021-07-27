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

repr::Program ana::InstallBuiltin::operator()(repr::Program& prog, pass::PassContext& ctx) {

    for (auto& cls : builtin::GetBuiltinClasses()) {
        if (!prog.AddClass(cls)) {
            ctx.diag.EmitError(prog.GetClassPtr(cls.name.val)->GetTextInfo(),
                "built-in class '" + cls.name.val + "' cannot be redefined");
            prog.InsertClass(cls);
        }
    }

    for (auto& cls : prog.GetClasses()) {
        if (cls->parent.Empty() && cls->name.val != "Object") cls->parent.val = "Object";
    }
    return prog;
}

repr::Class ana::CheckBuiltinInheritance::operator()(repr::Class& cls, pass::PassContext& ctx) {
    using namespace builtin;
    if (IsBuiltinClass(cls.parent.val) && !IsInheritable(cls.parent.val)) {
        ctx.diag.EmitError(cls.parent.textInfo, "cannot inherit built-in class '" + cls.parent.val + "'");
        cls.parent.val = "Object";
    }
    return cls;
}

repr::Program ana::BuildInheritanceTree::operator()(repr::Program& prog, pass::PassContext& ctx) {
    type::TypeAdvisor typeAdvisor(prog.GetClassPtr("Object"));

    vector<shared_ptr<repr::Class>> stack;

    auto emitCycleError = [](vector<shared_ptr<repr::Class>>& cycle, diag::Diagnosis& diag) {
        assert(cycle.size() > 1);
        string str = "cyclic inheritance detected: ";
        for (int i = 0; i < cycle.size() - 1; i++) {
            str += i == 0 ? "" : ", ";
            str += "'" + cycle.at(i)->name.val + "' inherits '" + cycle.at(i+1)->name.val + "'";
        }
        diag.EmitFatal(cycle.back()->GetTextInfo(), str);
    };

    auto add = [](vector<shared_ptr<repr::Class>>& classes, type::TypeAdvisor& typeAd) {
        while (!classes.empty()) {
            typeAd.AddType(classes.back());
            classes.pop_back();
        }
    };

    for (auto cls : prog.GetClasses()) {
        unordered_set<string> visiting;
        while (cls) {
            if (typeAdvisor.Contains(cls->name.val)) break;
            stack.emplace_back(cls);
            if (visiting.find(cls->name.val) != visiting.end()) {
                emitCycleError(stack, ctx.diag);
                return prog;
            }
            visiting.insert(cls->name.val);
            cls = prog.GetClassPtr(cls->parent.val);
        }
        add(stack, typeAdvisor);
    }


    ctx.Set<type::TypeAdvisor>("type_advisor", typeAdvisor);
    return prog;
}

repr::Class ana::CheckInheritedAttributes::operator()(repr::Class& cls, pass::PassContext& ctx) {
    auto& typeAdvisor = *ctx.Get<type::TypeAdvisor>("type_advisor");
    auto check = [&](shared_ptr<repr::Class> ancestor) {
        for (auto& field : cls.GetFieldFeatures()) {
            if (ancestor->GetFieldFeaturePtr(field->name.val)) {
                ctx.diag.EmitError(field->GetTextInfo(), "inherited attribute '" + field->name.val + "' cannot be redefined");
            }
        }
        return false;
    };
    typeAdvisor.BottomUpVisit(cls.parent.val, check);
    return cls;
}

repr::Class ana::AddInheritedAttributes::operator()(repr::Class& cls, pass::PassContext& ctx) {
    using namespace repr;
    auto& typeAdvisor = *ctx.Get<type::TypeAdvisor>("type_advisor");

    vector<shared_ptr<FieldFeature>> feats = cls.GetFieldFeatures();
    for (auto& feat : feats) cls.DeleteFieldFeature(feat->name.val);

    stack<shared_ptr<Class>> stack;
    shared_ptr<Class> cur = typeAdvisor.GetTypeRepr(cls.parent.val);
    while (cur) {
        stack.push(cur);
        cur = typeAdvisor.GetTypeRepr(cur->parent.val);
    }

    while (!stack.empty()) {
        for (auto& field : stack.top()->GetFieldFeatures()) cls.AddFieldFeature(*field);
        stack.pop();
    }

    for(auto& feat : feats) cls.AddFieldFeature(*feat);
    return cls;
}

repr::Class ana::CheckInheritedMethods::operator()(repr::Class& cls, pass::PassContext& ctx) {
    auto& typeAdvisor = *ctx.Get<type::TypeAdvisor>("type_advisor");
    auto valid = [](repr::FuncFeature& a, repr::FuncFeature& b) {
        if (a.name.val != b.name.val || a.type.val != b.type.val || a.args.size() != b.args.size()) return false;
        for(int i = 0; i < a.args.size(); i++) {
            if (a.args.at(i)->type.val != b.args.at(i)->type.val) return false;
        }
        return true;
    };
    auto check = [&](shared_ptr<repr::Class> ancestor) {
        for (auto& func : cls.GetFuncFeatures()) {
            if (ancestor->GetFuncFeaturePtr(func->name.val) &&
            !valid(*func, *(ancestor->GetFuncFeaturePtr(func->name.val)))) {
                ctx.diag.EmitError(func->GetTextInfo(), "invalid method overload: '" + func->name.val + "'");
            }
        }
        return false;
    };
    typeAdvisor.BottomUpVisit(cls.parent.val, check);
    return cls;
}

repr::Class ana::AddInheritedMethods::operator()(repr::Class& cls, pass::PassContext& ctx) {
    auto& typeAdvisor = *ctx.Get<type::TypeAdvisor>("type_advisor");

    auto cur = typeAdvisor.GetTypeRepr(cls.parent.val);
    while (cur) {
        for (auto& func : cur->GetFuncFeatures())
            cls.AddFuncFeature(*func);
        cur = typeAdvisor.GetTypeRepr(cur->parent.val);
    }

    return cls;
}

repr::Program ana::InitSymbolTable::operator()(repr::Program& prog, pass::PassContext& ctx) {
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
                for (auto &cls : prog.GetClasses()) Visit(cls);
            }, nullptr)
        }

        void Visit(shared_ptr<repr::Class> cls) {
            NEW_SCOPE_GUARD(stable, {
                for (auto &feat : cls->GetFuncFeatures()) Visit(*feat);
                for (int i = 0; i < cls->GetFieldFeatures().size(); i++)
                    Visit(*cls->GetFieldFeatures().at(i), i);
            }, cls)
        }

        void Visit(repr::FuncFeature &feat) {
            NEW_SCOPE_GUARD(stable, {
                for (int i = 0; i < feat.args.size(); i++)
                    Visit(*feat.args.at(i), i);
                ExprVisitor::Visit(*feat.expr);
            }, stable.GetClass())
        }

        void Visit(repr::FieldFeature &feat, int idx) {
            stable.Current().Insert(IdAttr{IdAttr::Field, idx, feat.name.val, feat.type.val});
            if (feat.expr) ExprVisitor::Visit(*feat.expr);
        }

        void Visit(repr::Formal &form, int idx) {
            stable.Current().Insert(IdAttr{IdAttr::Arg, idx, form.name.val, form.type.val});
        }

        void Visit_(repr::LinkBuiltin& expr) {}

        void Visit_(repr::Assign& expr) {}

        void Visit_(repr::Add& expr) { VisitBinary(expr); }

        void Visit_(repr::Block& expr) {
            NEW_SCOPE_GUARD(stable, {
                for (auto &x : expr.exprs) ExprVisitor::Visit(*x);
            }, stable.GetClass())
        }

        void Visit_(repr::Case& expr) {
            ExprVisitor::Visit(*expr.expr);
            for (auto& branch : expr.branches) {
                NEW_SCOPE_GUARD(stable, {
                    stable.Current().Insert(IdAttr{IdAttr::Local, stable.Current().NextLocalIdIdx(),
                                                   branch->id.val, branch->type.val});
                    ExprVisitor::Visit(*branch->expr);
                }, stable.GetClass())
            }
        }

        // this function does not take care of scope
        void VisitCall(repr::Call& expr) {
            Visit_(*expr.id);
            for (auto &arg : expr.args)
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
            ExprVisitor::Visit(*expr.expr);
        }
        void Visit_(repr::Integer& expr) { return; }

        void Visit_(repr::If& expr) {
            NEW_SCOPE_GUARD(stable,{
                ExprVisitor::Visit(*expr.ifExpr);
                NEW_SCOPE_GUARD(stable, ExprVisitor::Visit(*expr.thenExpr), stable.GetClass());
                NEW_SCOPE_GUARD(stable, ExprVisitor::Visit(*expr.elseExpr), stable.GetClass());
            }, stable.GetClass())
        }

        void Visit_(repr::LessThanOrEqual& expr) { VisitBinary(expr); }
        void Visit_(repr::LessThan& expr) { VisitBinary(expr); }

        void Visit_(repr::Let& expr) {
            for (int i = 0; i < expr.formals.size(); i++) {
                stable.NewScope(stable.GetClass());
                auto& form = expr.formals.at(i);
                stable.Current().Insert(IdAttr{IdAttr::Local, stable.Current().NextLocalIdIdx(),
                                               form->name.val, form->type.val});
                if (form->expr) ExprVisitor::Visit(*form->expr);
            }
            ExprVisitor::Visit(*expr.expr);
            for (int i = 0; i < expr.formals.size(); i++) stable.FinishScope();
        }

        void Visit_(repr::MethodCall& expr) {
            NEW_SCOPE_GUARD(stable, {
                ExprVisitor::Visit(*expr.left);
                VisitCall(*static_pointer_cast<repr::Call>(expr.right));
            }, stable.GetClass())
        }

        void Visit_(repr::Multiply& expr) { VisitBinary(expr); }
        void Visit_(repr::Minus& expr) { VisitBinary(expr); }

        void Visit_(repr::Negate& expr) {
            ExprVisitor::Visit(*expr.expr);
        }
        void Visit_(repr::New& expr) { return; }

        void Visit_(repr::Not& expr) {
            ExprVisitor::Visit(*expr.expr);
        }

        void Visit_(repr::String& expr) { return; }
        void Visit_(repr::True& expr) { return; }
        void Visit_(repr::While& expr) {
            NEW_SCOPE_GUARD(stable, {
                ExprVisitor::Visit(*expr.whileExpr);
                ExprVisitor::Visit(*expr.loopExpr);
            }, stable.GetClass())
        }

        void VisitBinary(repr::Binary& expr) {
            ExprVisitor::Visit(*expr.left);
            ExprVisitor::Visit(*expr.right);
        }
    };

    Visitor vis;
    vis.Visit(prog);
    ctx.Set<ScopedTableSpecializer<SymbolTable>>("symbol_table", vis.stable);
    return prog;
}

repr::Program ana::TypeChecking::operator()(repr::Program& prog, pass::PassContext& ctx) {
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
                auto& p = cls.parent;
                for (auto& feat : cls.GetFieldFeatures()) Visit(*feat);
                for (auto& feat : cls.GetFuncFeatures()) Visit(*feat);
            })
        }

        void Visit(repr::FieldFeature &feat) {
            if (!feat.expr) return;
            TypeName exprType = ExprVisitor<TypeName>::Visit(*feat.expr);
            if (!typeAdvisor.Conforms(exprType, feat.type.val, stable.GetClass()->name.val))
                ctx.diag.EmitError(feat.GetTextInfo(), invalidAssignmentMsg(exprType, feat.type.val));
        }

        void Visit(repr::FuncFeature &feat) {
            ENTER_SCOPE_GUARD(stable, {
                TypeName exprType = ExprVisitor<TypeName>::Visit(*feat.expr);
                if (!typeAdvisor.Conforms(exprType, feat.type.val, stable.GetClass()->name.val))
                    ctx.diag.EmitError(feat.GetTextInfo(), invalidAssignmentMsg(exprType, feat.type.val));
            })
        }

        void Visit(repr::Formal &form) {}

        TypeName Visit_(repr::LinkBuiltin& expr) { return expr.type; }

        TypeName Visit_(repr::Assign& expr) {
            auto exprType = ExprVisitor<TypeName>::Visit(*expr.expr);
            auto idAttr = stable.GetIdAttr(expr.id->name.val);
            if (!idAttr)
                ctx.diag.EmitError(expr.id->GetTextInfo(), idNotFound(expr.id->name.val));
            else if (!typeAdvisor.Conforms(exprType, idAttr->type, stable.GetClass()->name.val))
                ctx.diag.EmitError(expr.expr->GetTextInfo(), invalidAssignmentMsg(exprType, idAttr->type));
            return exprType;
        }

        TypeName Visit_(repr::Add& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.left) != "Int")
                ctx.diag.EmitError(expr.left->GetTextInfo(), "operand for '+' must be 'Int'");
            if (ExprVisitor<TypeName>::Visit(*expr.right) != "Int")
                ctx.diag.EmitError(expr.right->GetTextInfo(), "operand for '+' must be 'Int'");
            return "Int";
        }

        TypeName Visit_(repr::Block& expr) {
            TypeName rType;
            ENTER_SCOPE_GUARD(stable, {
                for (int i = 0; i < expr.exprs.size() - 1; i++) ExprVisitor<TypeName>::Visit(*expr.exprs.at(i));
                rType = ExprVisitor<TypeName>::Visit(*expr.exprs.back());
            })
            return rType;;
        }

        TypeName Visit_(repr::Case& expr) {
            unordered_set<TypeName> typeSet;
            vector<TypeName> types;
            for (auto& branch : expr.branches) {
                ENTER_SCOPE_GUARD(stable, {
                    auto type = ExprVisitor<TypeName>::Visit(*branch->expr);
                    if (typeSet.find(type) != typeSet.end())
                        ctx.diag.EmitError(branch->type.textInfo, "duplicate type '" + type + "' in case expression");
                    typeSet.insert(type);
                    types.emplace_back(type);
                })
            }
            return typeAdvisor.LeastCommonAncestor(types);
        }

        TypeName Visit_(repr::Call& expr) {
            TypeName rType;
            ENTER_SCOPE_GUARD(stable,
                auto funcPtr = CheckCall(stable.GetClass()->name.val, expr);
                if (funcPtr) {
                    expr.link = funcPtr;
                    rType = funcPtr->type.val;
                })
            return rType;
        }

        TypeName Visit_(repr::MethodCall& expr) {
            TypeName rType;
            ENTER_SCOPE_GUARD(stable,
                auto callExpr = static_pointer_cast<repr::Call>(expr.right);
                expr.type = ExprVisitor<TypeName>::Visit(*expr.left);
                auto funcPtr = CheckCall(expr.type, *callExpr);
                if (funcPtr) {
                    callExpr->link = funcPtr;
                    rType = funcPtr->type.val;
                })
            return rType;
        }

        shared_ptr<repr::FuncFeature> CheckCall(TypeName type, repr::Call& expr) {
            auto cls = typeAdvisor.GetTypeRepr(type);
            if (!cls) {
                ctx.diag.EmitError(expr.GetTextInfo(), "caller type '" + type + "' not found");
                return nullptr;
            }
            auto funcPtr = cls->GetFuncFeaturePtr(expr.id->name.val);
            if (!funcPtr) {
                ctx.diag.EmitError(expr.GetTextInfo(), "method '" + expr.id->name.val + "' not found");
                return nullptr;
            }
            if (expr.args.size() != funcPtr->args.size()) {
                ctx.diag.EmitError(expr.GetTextInfo(), "expected " + to_string(funcPtr->args.size()) +
                " arguments, got " + to_string(expr.args.size()));
                return funcPtr;
            }
            for (int i = 0; i < expr.args.size(); i++) {
                auto& arg = expr.args.at(i);
                TypeName got = ExprVisitor<TypeName>::Visit(*arg);
                TypeName expected = funcPtr->args.at(i)->type.val;
                if (!typeAdvisor.Conforms(got, expected, got)) {
                    ctx.diag.EmitError(expr.GetTextInfo(), "invalid argument '" + funcPtr->args.at(i)->name.val +
                    "': expected '" + expected + "', got '" + got + "'");
                    return funcPtr;
                }
            }
            return funcPtr;
        }

        TypeName Visit_(repr::Divide& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.left) != "Int")
                ctx.diag.EmitError(expr.left->GetTextInfo(), "operand for '/' must be 'Int'");
            if (ExprVisitor<TypeName>::Visit(*expr.right) != "Int")
                ctx.diag.EmitError(expr.right->GetTextInfo(), "operand for '/' must be 'Int'");
            return "Int";
        }

        TypeName Visit_(repr::Equal& expr) {
            auto leftType = ExprVisitor<TypeName>::Visit(*expr.left);
            auto rightType = ExprVisitor<TypeName>::Visit(*expr.right);
            if (((leftType == "Int" || leftType == "String" || leftType == "Bool") ||
                (rightType == "Int" || rightType == "String" || rightType == "Bool")) &&
                (leftType != rightType))
                ctx.diag.EmitError(expr.right->GetTextInfo(), "'Int', 'String', 'Bool' can only be compared with the same type");
            return "Bool";
        }

        TypeName Visit_(repr::False& expr) { return "Bool"; }

        TypeName Visit_(repr::ID& expr) {
            auto idAttr = stable.GetIdAttr(expr.name.val);
            if (!idAttr) {
                ctx.diag.EmitError(expr.GetTextInfo(), idNotFound(expr.name.val));
                return "";
            }
            return idAttr->type;
        }

        TypeName Visit_(repr::IsVoid& expr) {
            ExprVisitor<TypeName>::Visit(*expr.expr);
            return "Bool";
        }

        TypeName Visit_(repr::Integer& expr) { return "Int"; }

        TypeName Visit_(repr::If& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.ifExpr) != "Bool")
                ctx.diag.EmitError(expr.ifExpr->GetTextInfo(), "predicate in if statement must be 'Bool'");
            return typeAdvisor.LeastCommonAncestor(
                ExprVisitor<TypeName>::Visit(*expr.thenExpr),
                ExprVisitor<TypeName>::Visit(*expr.elseExpr));
        }

        TypeName Visit_(repr::LessThanOrEqual& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.left) != "Int")
                ctx.diag.EmitError(expr.left->GetTextInfo(), "operand for '<=' must be 'Int'");
            if (ExprVisitor<TypeName>::Visit(*expr.right) != "Int")
                ctx.diag.EmitError(expr.right->GetTextInfo(), "operand for '<=' must be 'Int'");
            return "Bool";
        }

        TypeName Visit_(repr::LessThan& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.left) != "Int")
                ctx.diag.EmitError(expr.left->GetTextInfo(), "operand for '<' must be 'Int'");
            if (ExprVisitor<TypeName>::Visit(*expr.right) != "Int")
                ctx.diag.EmitError(expr.right->GetTextInfo(), "operand for '<' must be 'Int'");
            return "Bool";
        }

        TypeName Visit_(repr::Let& expr) {
            TypeName rType;
            for (int i = 0; i < expr.formals.size(); i++) {
                stable.EnterScope();
                auto& form = expr.formals.at(i);
                if (form->expr) {
                    auto exprType = ExprVisitor<TypeName>::Visit(*form->expr);
                    if (!typeAdvisor.Conforms(exprType, form->type.val, stable.GetClass()->name.val))
                        ctx.diag.EmitError(form->expr->GetTextInfo(), invalidAssignmentMsg(exprType, form->type.val));
                }
            }
            rType = ExprVisitor<TypeName>::Visit(*expr.expr);
            for (int i = 0; i < expr.formals.size(); i++) stable.LeaveScope();
            return rType;
        }

        TypeName Visit_(repr::Multiply& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.left) != "Int")
                ctx.diag.EmitError(expr.left->GetTextInfo(), "operand for '*' must be 'Int'");
            if (ExprVisitor<TypeName>::Visit(*expr.right) != "Int")
                ctx.diag.EmitError(expr.right->GetTextInfo(), "operand for '*' must be 'Int'");
            return "Int";
        }

        TypeName Visit_(repr::Minus& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.left) != "Int")
                ctx.diag.EmitError(expr.left->GetTextInfo(), "operand for '-' must be 'Int'");
            if (ExprVisitor<TypeName>::Visit(*expr.right) != "Int")
                ctx.diag.EmitError(expr.right->GetTextInfo(), "operand for '-' must be 'Int'");
            return "Int";
        }

        TypeName Visit_(repr::Negate& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.expr) != "Int")
                ctx.diag.EmitError(expr.GetTextInfo(), "operand for '~' must be 'Int'");
            return "Int";
        }

        TypeName Visit_(repr::New& expr) {
            return expr.type.val;
        }

        TypeName Visit_(repr::Not& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.expr) != "Bool")
                ctx.diag.EmitError(expr.GetTextInfo(), "operand for 'not' must be 'Int'");
            return "Bool";
        }

        TypeName Visit_(repr::String& expr) { return "String"; }

        TypeName Visit_(repr::True& expr) { return "Bool"; }

        TypeName Visit_(repr::While& expr) {
            ENTER_SCOPE_GUARD(stable, {
                if (ExprVisitor<TypeName>::Visit(*expr.whileExpr) != "Bool")
                    ctx.diag.EmitError(expr.GetTextInfo(), "predicate in while expression must be 'Bool'");
                ExprVisitor<TypeName>::Visit(*expr.loopExpr);
            })
            return "Object";
        }
    };

    auto stable = ctx.Get<ScopedTableSpecializer<SymbolTable>>("symbol_table");
    auto typeAdvisor = ctx.Get<type::TypeAdvisor>("type_advisor");

    Visitor vis(ctx, *stable, *typeAdvisor);
    vis.Visit(prog);
    return prog;
}

repr::Program ana::EliminateSelfType::operator()(repr::Program& prog, pass::PassContext& ctx) {

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
                auto& p = cls.parent;
                for (auto& feat : cls.GetFieldFeatures()) Visit(*feat);
                for (auto& feat : cls.GetFuncFeatures()) Visit(*feat);
            })
        }

        void Visit(repr::FieldFeature &feat) {
            if (feat.type.val == "SELF_TYPE")
                feat.type.val = stable.GetClass()->name.val;
            if (feat.expr)
                Visit(*feat.expr);
        }

        void Visit(repr::FuncFeature &feat) {
            ENTER_SCOPE_GUARD(stable, {
                if (feat.type.val == "SELF_TYPE")
                    feat.type.val = stable.GetClass()->name.val;
                Visit(*feat.expr);
            })
        }

        void Visit(repr::Formal &form) {}

        void Visit(repr::Expr& expr) { ExprVisitor<void>::Visit(expr); }

        void Visit_(repr::LinkBuiltin& expr) {
            if (expr.type == "SELF_TYPE")
                expr.type = stable.GetClass()->name.val;
        }

        void Visit_(repr::Assign& expr) { Visit(*expr.expr); }

        void Visit_(repr::Add& expr) {
            Visit(*expr.left);
            Visit(*expr.left);
        }

        void Visit_(repr::Block& expr) {
            ENTER_SCOPE_GUARD(stable, {
                for (int i = 0; i < expr.exprs.size() - 1; i++) Visit(*expr.exprs.at(i));
            })
        }

        void Visit_(repr::Case& expr) {
            Visit(*expr.expr);
            for (auto& branch : expr.branches) {
                ENTER_SCOPE_GUARD(stable, Visit(*branch->expr);)
            }
        }

        void VisitCall(repr::Call& expr) {
            for (auto& arg : expr.args) Visit(*arg);
        }

        void Visit_(repr::Call& expr) {
            ENTER_SCOPE_GUARD(stable,VisitCall(expr))
        }

        void Visit_(repr::MethodCall& expr) {
            ENTER_SCOPE_GUARD(stable, {
                Visit(*expr.left);
                VisitCall(*static_pointer_cast<repr::Call>(expr.right));
            })
        }

        void Visit_(repr::Divide& expr) {
            Visit(*expr.left);
            Visit(*expr.right);
        }

        void Visit_(repr::Equal& expr) {
            Visit(*expr.left);
            Visit(*expr.right);
        }

        void Visit_(repr::False& expr) {}

        void Visit_(repr::ID& expr) {}

        void Visit_(repr::IsVoid& expr) { Visit(*expr.expr); }

        void Visit_(repr::Integer& expr) {}

        void Visit_(repr::If& expr) {
            Visit(*expr.ifExpr);
            Visit(*expr.thenExpr);
            Visit(*expr.elseExpr);
        }

        void Visit_(repr::LessThanOrEqual& expr) {
            Visit(*expr.left);
            Visit(*expr.right);
        }

        void Visit_(repr::LessThan& expr) {
            Visit(*expr.left);
            Visit(*expr.right);
        }

        void Visit_(repr::Let& expr) {
            for (int i = 0; i < expr.formals.size(); i++) {
                stable.EnterScope();
                Visit(*expr.formals.at(i)->expr);
            }
            Visit(*expr.expr);
            for (int i = 0; i < expr.formals.size(); i++) stable.LeaveScope();
        }

        void Visit_(repr::Multiply& expr) {
            Visit(*expr.left);
            Visit(*expr.right);
        }

        void Visit_(repr::Minus& expr) {
            Visit(*expr.left);
            Visit(*expr.right);
        }

        void Visit_(repr::Negate& expr) {
            Visit(*expr.expr);
        }

        void Visit_(repr::New& expr) {
            if (expr.type.val == "SELF_TYPE")
                expr.type.val = stable.GetClass()->name.val;
        }

        void Visit_(repr::Not& expr) { Visit(*expr.expr); }

        void Visit_(repr::String& expr) {}

        void Visit_(repr::True& expr) {}

        void Visit_(repr::While& expr) {
            ENTER_SCOPE_GUARD(stable, {
                Visit(*expr.whileExpr);
                Visit(*expr.loopExpr);
            })
        }
    };

    auto stable = ctx.Get<ScopedTableSpecializer<SymbolTable>>("symbol_table");
    Visitor vis(*stable);
    vis.Visit(prog);
    return prog;
}