//
// Created by 田地 on 2021/6/14.
//

#include <algorithm>

#include "analysis.h"
#include "builtin.h"
#include "pass.h"
#include "stable.h"
#include "visitor.h"
#include "typead.h"
#include "repr.h"
#include "token.h"

using namespace std;
using namespace cool;

repr::Program ana::InstallBuiltin::operator()(repr::Program& prog, pass::PassContext& ctx) {
    prog.classes.emplace_back(make_shared<repr::Class>(builtin::Object));
    prog.classes.emplace_back(make_shared<repr::Class>(builtin::IO));
    prog.classes.emplace_back(make_shared<repr::Class>(builtin::Int));
    prog.classes.emplace_back(make_shared<repr::Class>(builtin::String));
    prog.classes.emplace_back(make_shared<repr::Class>(builtin::Bool));

    for (auto& cls : prog.classes) {
        if (cls->parent.Empty() && cls->name.val != "Object") cls->parent.val = "Object";
    }
    return prog;
}

repr::Program ana::InitSymbolTable::operator()(repr::Program& prog, pass::PassContext& ctx) {
    using namespace visitor;

    // todo: attribute redefinition checker
    class ClassRedefineChecker : public ProgramVisitor {
      public:
        pass::PassContext& ctx;

        ClassRedefineChecker(pass::PassContext& _ctx) : ctx(_ctx) {}

        void Visit(repr::Program &prog) {
            unordered_map<string, shared_ptr<repr::Class>> classes;
            for (auto it = prog.classes.begin(); it != prog.classes.end(); ) {
                auto cls = *it;
                string name = cls->name.val;
                if (classes.find(name) != classes.end()) {
                    if (builtin::BuiltinClassSet.find(name) != builtin::BuiltinClassSet.end())
                        ctx.diag.EmitError(cls->GetTextInfo(), "built-in class '" + name + "' cannot be redefined");
                    else
                        ctx.diag.EmitError(cls->GetTextInfo(),
                            "class '" + cls->name.val +"' redefined, see previous declararion at: " +
                            classes.at(name)->GetTextInfo().String());
                    prog.classes.erase(it);
                } else {
                    classes.insert({cls->name.val, cls});
                    it++;
                }
            }
        }
    };

    class FuncRedefineChecker : public ProgramVisitor, ClassVisitor {
      public:
        pass::PassContext& ctx;

        FuncRedefineChecker(pass::PassContext& _ctx) : ctx(_ctx) {}

        void Visit(repr::Program &prog) {
            for (auto& cls : prog.classes) Visit(*cls);
        }

        // todo: fix func redefinition bug
        void Visit(repr::Class &cls) {
            unordered_map<string, shared_ptr<repr::FuncFeature>> funcs;
            for (auto it = cls.funcs.begin(); it != cls.funcs.end(); ) {
                auto func = *it;
                if (funcs.find(func->name.val) != funcs.end()) {
                    ctx.diag.EmitError(func->GetTextInfo(),
                        "function '" + func->name.val + "' redefined, see previous declaration at line: " +
                            funcs.at(func->name.val)->GetTextInfo().String());
                    cls.funcs.erase(it);
                } else {
                    funcs.insert({func->name.val, func});
                    it++;
                }
            }
        }
    };

    class Visitor : public ProgramVisitor, ClassVisitor, FuncFeatureVisitor,
        FieldFeatureVisitor, FormalVisitor, ExprVisitor<void> {
      public:
        ScopedTableSpecializer<SymbolTable> stable;

        void Visit(repr::Program &prog) {
            NEW_SCOPE_GUARD(stable, {
                for (auto &cls : prog.classes) Visit(*cls);
            }, nullptr)
        }

        void Visit(repr::Class &cls) {
            NEW_SCOPE_GUARD(stable, {
                for (auto &feat : cls.funcs) Visit(*feat);
                for (auto &feat : cls.fields) Visit(*feat);
            }, make_shared<repr::Class>(cls))
        }

        void Visit(repr::FuncFeature &feat) {
            NEW_SCOPE_GUARD(stable, {
                for (auto &arg : feat.args) Visit(*arg);
                ExprVisitor::Visit(*feat.expr);
            }, stable.GetClass())
        }

        void Visit(repr::FieldFeature &feat) {
            stable.Current().Insert(attr::IdAttr{feat.name.val, feat.type.val});
            if (feat.expr) ExprVisitor::Visit(*feat.expr);
        }

        void Visit(repr::Formal &form) {
            stable.Current().Insert(attr::IdAttr{form.name.val, form.type.val});
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
                    stable.Current().Insert(attr::IdAttr{branch->id.val, branch->type.val});
                    ExprVisitor::Visit(*branch->expr);
                }, stable.GetClass())
            }
        }

        void Visit_(repr::Call& expr) {
            NEW_SCOPE_GUARD(stable, {
                Visit_(*expr.id);
                for (auto &arg : expr.args) { ExprVisitor::Visit(*arg); }
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
                stable.Current().Insert(attr::IdAttr{form->name.val, form->type.val});
                if (form->expr) ExprVisitor::Visit(*form->expr);
            }
            ExprVisitor::Visit(*expr.expr);
            for (int i = 0; i < expr.formals.size(); i++) stable.FinishScope();
        }

        void Visit_(repr::MethodCall& expr) {
            NEW_SCOPE_GUARD(stable, {
                ExprVisitor::Visit(*expr.left);
                ExprVisitor::Visit(*expr.right);
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

    ClassRedefineChecker clsChecker(ctx);
    clsChecker.Visit(prog);
    FuncRedefineChecker funcChecker(ctx);
    funcChecker.Visit(prog);
    Visitor vis;
    vis.Visit(prog);
    ctx.Set<ScopedTableSpecializer<SymbolTable>>("symbol_table", vis.stable);
    return prog;
}

void ana::InitSymbolTable::Required() { pass::PassManager::Required<InitSymbolTable, InstallBuiltin>(); }

repr::Program ana::BuildInheritanceTree::operator()(repr::Program& prog, pass::PassContext& ctx) {
    using namespace visitor;
    using namespace builtin;

    class Checker : public ProgramVisitor, ClassVisitor {
      public:
        pass::PassContext& ctx;

        Checker(pass::PassContext& _ctx) : ctx(_ctx) {}

        void Visit(repr::Program &prog) { for (auto &cls : prog.classes) Visit(*cls); }

        void Visit(repr::Class &cls) {
            if (BuiltinClassSet.find(cls.parent.val) != BuiltinClassSet.end() &&
                InheritableBuiltInClasses.find(cls.parent.val) == InheritableBuiltInClasses.end() &&
                cls.parent.val != "Object") {
                ctx.diag.EmitError(cls.parent.textInfo, "cannot inherit built-in class '" + cls.parent.val + "'");
                cls.parent.val = "Object";
            }
        }
    };

    class Visitor : public ProgramVisitor, ClassVisitor {
      public:
        pass::PassContext& ctx;
        ScopedTableSpecializer<SymbolTable>& stable;
        type::TypeAdvisor typeAdvisor;
        unordered_map<string, shared_ptr<repr::Class>> map;

        Visitor(pass::PassContext& _ctx, ScopedTableSpecializer<SymbolTable>& _stable) : ctx(_ctx), stable(_stable),
        typeAdvisor(make_shared<repr::Class>(builtin::Object)) {
            stable.InitTraverse();
        }

        void Visit(repr::Program& prog) {
            ENTER_SCOPE_GUARD(stable, {
                for (auto& cls : prog.classes) map.insert({cls->name.val, cls});
                for (auto& cls : prog.classes) Visit(cls);
            })
        }

        void Visit(shared_ptr<repr::Class> cls) {
            if (!typeAdvisor.Contains(cls->name.val)) {
                vector<shared_ptr<repr::Class>> sorted;
                while (!typeAdvisor.Contains(cls->name.val)) {
                    sorted.emplace_back(cls);
                    if (cls->parent.Empty()) break;
                    cls = map.at(cls->parent.val);
                }
                for (auto it = sorted.rbegin(); it != sorted.rend(); it++) {
                    typeAdvisor.AddType(*it);
                }
            }
        }
    };

    class InheritedAttrMutator : public ProgramVisitor, ClassVisitor {
      public:
        ScopedTableSpecializer<SymbolTable>& stable;
        type::TypeAdvisor& typeAdvisor;

        InheritedAttrMutator(ScopedTableSpecializer<SymbolTable>& _stable, type::TypeAdvisor& _typeAdvisor)
        : stable(_stable), typeAdvisor(_typeAdvisor) {
            stable.InitTraverse();
        }

        void Visit(repr::Program& prog) {
            ENTER_SCOPE_GUARD(stable, for (auto& cls : prog.classes) Visit(*cls);)
        }

        void Visit(repr::Class &cls) {
            auto ancestor = typeAdvisor.GetTypeRepr(cls.parent.val);
            while (ancestor) {
                for (auto& field : ancestor->fields) {
                    if (stable.GetIdAttr(field->name.val)) continue;
                    if (field->type.val == "SELF_TYPE") stable.Insert(attr::IdAttr{field->name.val, cls.name.val});
                    else stable.Insert(attr::IdAttr{field->name.val, field->type.val});
                }
                ancestor = typeAdvisor.GetTypeRepr(ancestor->parent.val);
            }
        }
    };

    auto stable = ctx.Get<ScopedTableSpecializer<SymbolTable>>("symbol_table");
    Checker chek(ctx);
    chek.Visit(prog);
    Visitor vis(ctx, *stable);
    vis.Visit(prog);
    InheritedAttrMutator mutator(*stable, vis.typeAdvisor);
    mutator.Visit(prog);
    ctx.Set<type::TypeAdvisor>("type_advisor", vis.typeAdvisor);
    return prog;
}

void ana::BuildInheritanceTree::Required() { pass::PassManager::Required<BuildInheritanceTree, InitSymbolTable>(); }

repr::Program ana::TypeChecking::operator()(repr::Program& prog, pass::PassContext& ctx) {
    using namespace visitor;
    using namespace builtin;
    using namespace tok;

    using TypeName = string;

    class Visitor : public ProgramVisitor, ClassVisitor, FuncFeatureVisitor,
        FieldFeatureVisitor, FormalVisitor, ExprVisitor<TypeName> {
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
            ENTER_SCOPE_GUARD(stable,for (auto& cls : prog.classes) Visit(*cls);)
        }

        void Visit(repr::Class &cls) {
            ENTER_SCOPE_GUARD(stable, {
                auto& p = cls.parent;
                for (auto& feat : cls.fields) Visit(*feat);
                for (auto& feat : cls.funcs) Visit(*feat);
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
            ENTER_SCOPE_GUARD(stable, rType = CheckCall(stable.GetClass()->name.val, expr);)
            return rType;
        }

        TypeName Visit_(repr::MethodCall& expr) {
            TypeName rType;
            ENTER_SCOPE_GUARD(stable,
                rType = CheckCall(ExprVisitor<TypeName>::Visit(*expr.left), *static_pointer_cast<repr::Call>(expr.right));)
            return rType;
        }

        TypeName CheckCall(TypeName type, repr::Call& expr) {
            TypeName rtype = "";
            auto dispatch = [&](shared_ptr<repr::Class> cls) {
                if (!cls) throw runtime_error("class pointer cannot be nullptr");
                auto it = find_if(cls->funcs.begin(), cls->funcs.end(),
                                  [&expr](shared_ptr<repr::FuncFeature>& func){ return func->name.val == expr.id->name.val; });
                if (it == cls->funcs.end()) return false;
                if (expr.args.size() != (*it)->args.size()) return false;
                for (int i = 0; i < expr.args.size(); i++) {
                    auto& arg = expr.args.at(i);
                    TypeName type = ExprVisitor<TypeName>::Visit(*arg);
                    TypeName expected = (*it)->args.at(i)->type.val;
                    if (!typeAdvisor.Conforms(type, expected, type)) return false;
                }
                rtype = (*it)->type.val;
                return true;
            };
            typeAdvisor.BottomUpVisit(type, dispatch);
            if (rtype.empty())
                ctx.diag.EmitError(expr.id->GetTextInfo(),"no available dispatch found");
            return rtype;
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

        TypeName Visit_(repr::IsVoid& expr) { return "Bool"; }

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

void ana::TypeChecking::Required() { pass::PassManager::Required<TypeChecking, BuildInheritanceTree>(); }
