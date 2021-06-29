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

using namespace std;
using namespace cool;

repr::Program ana::InstallBuiltin::operator()(repr::Program& prog, pass::PassContext& ctx) {
    prog.classes.emplace_back(make_shared<repr::Class>(builtin::Object));
    prog.classes.emplace_back(make_shared<repr::Class>(builtin::IO));
    prog.classes.emplace_back(make_shared<repr::Class>(builtin::Int));
    prog.classes.emplace_back(make_shared<repr::Class>(builtin::String));
    prog.classes.emplace_back(make_shared<repr::Class>(builtin::Bool));

    using namespace visitor;

    class Visitor : public ClassVisitor {
      public:
        void Visit(repr::Class &cls) {
            if (cls.parent.empty() && cls.name != "Object") {
                cls.parent = "Object";
            }
        }
    };

    Visitor vis;
    for (auto& cls : prog.classes) {
        vis.Visit(*cls);
    }
    return prog;
}

repr::Program ana::InitSymbolTable::operator()(repr::Program& prog, pass::PassContext& ctx) {
    using namespace visitor;

    class Visitor : public ProgramVisitor, ClassVisitor, FuncFeatureVisitor,
        FieldFeatureVisitor, FormalVisitor, ExprVisitor<void> {
      public:
        ScopedTableSpecializer<SymbolTable> stable;

        void Visit(repr::Program &prog) {
            NEW_SCOPE_GUARD(stable, {
                for (auto &cls : prog.classes) { Visit(*cls); }
            }, nullptr)
        }

        void Visit(repr::Class &cls) {
            NEW_SCOPE_GUARD(stable, {
                for (auto &feat : cls.funcs) { Visit(*feat); }
                for (auto &feat : cls.fields) { Visit(*feat); }
            }, make_shared<repr::Class>(cls))
        }

        void Visit(repr::FuncFeature &feat) {
            NEW_SCOPE_GUARD(stable, {
                for (auto &form : feat.args) { Visit(*form); }
                if (feat.expr) ExprVisitor::Visit(*feat.expr);
            }, stable.GetClass())
        }

        void Visit(repr::FieldFeature &feat) {
            stable.Current().Insert(attr::IdAttr{feat.name, feat.type});
            if (feat.expr) ExprVisitor::Visit(*feat.expr);
        }

        void Visit(repr::Formal &form) {
            stable.Current().Insert(attr::IdAttr{form.name, form.type});
        }

        void Visit_(repr::Assign& expr) {
            stable.Current().Insert(attr::IdAttr{expr.id->name, ""});
        }
        void Visit_(repr::Add& expr) { VisitBinary(expr); }
        void Visit_(repr::Block& expr) {
            NEW_SCOPE_GUARD(stable, {
                for (auto &x : expr.exprs) { ExprVisitor::Visit(*x); }
            }, stable.GetClass())
        }
        void Visit_(repr::Case& expr) {
            ExprVisitor::Visit(*expr.expr);
            for (auto& branch : expr.branches) {
                NEW_SCOPE_GUARD(stable, {
                    stable.Current().Insert(attr::IdAttr{branch->id, branch->type});
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
        void Visit_(repr::IsVoid& expr) { ExprVisitor::Visit(*expr.expr); }
        void Visit_(repr::Integer& expr) { return; }
        void Visit_(repr::If& expr) {
            NEW_SCOPE_GUARD(stable,{
                ExprVisitor::Visit(*expr.ifExpr);
                NEW_SCOPE_GUARD(stable, *expr.thenExpr, stable.GetClass());
                NEW_SCOPE_GUARD(stable, *expr.elseExpr, stable.GetClass());
            }, stable.GetClass())
        }
        void Visit_(repr::LessThanOrEqual& expr) { VisitBinary(expr); }
        void Visit_(repr::LessThan& expr) { VisitBinary(expr); }
        void Visit_(repr::Let& expr) {
            for (int i = 0; i < expr.formals.size(); i++) {
                NEW_SCOPE_GUARD(stable, {
                    auto& form = expr.formals.at(i);
                    stable.Current().Insert(attr::IdAttr{form->name, form->type});
                    if (form->expr) {
                        ExprVisitor::Visit(*form->expr);
                    }
                    if (i == expr.formals.size()-1) {
                        ExprVisitor::Visit(*expr.expr);
                    }
                }, stable.GetClass())
            }
        }
        void Visit_(repr::MethodCall& expr) {
            NEW_SCOPE_GUARD(stable, {
                ExprVisitor::Visit(*expr.left);
                ExprVisitor::Visit(*expr.right);
            }, stable.GetClass())
        }
        void Visit_(repr::Multiply& expr) { VisitBinary(expr); }
        void Visit_(repr::Minus& expr) { VisitBinary(expr); }
        void Visit_(repr::Negate& expr) { ExprVisitor::Visit(*expr.expr); }
        void Visit_(repr::New& expr) { return; }
        void Visit_(repr::Not& expr) { ExprVisitor::Visit(*expr.expr); }
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

void ana::InitSymbolTable::Required() { pass::PassManager::Required<InitSymbolTable, InstallBuiltin>(); }

repr::Program ana::BuildInheritanceTree::operator()(repr::Program& prog, pass::PassContext& ctx) {
    class Visitor : public visitor::ProgramVisitor, visitor::ClassVisitor {
      public:
        ScopedTableSpecializer<SymbolTable>& stable;
        TypeAdvisor typeAdvisor;
        unordered_map<string, shared_ptr<repr::Class>> map;

        Visitor(ScopedTableSpecializer<SymbolTable>& _stable) : stable(_stable), typeAdvisor("Object") {
            stable.InitTraverse();
        }

        void Visit(repr::Program& prog) {
            ENTER_SCOPE_GUARD(stable, {
                for (auto& cls : prog.classes) map.insert({cls->name, cls});
                for (auto& cls : prog.classes) Visit(cls);
            })
        }

        void Visit(shared_ptr<repr::Class> cls) {
            if (!typeAdvisor.Contains(cls->name)) {
                vector<shared_ptr<repr::Class>> sorted;
                while (!typeAdvisor.Contains(cls->name)) {
                    sorted.emplace_back(cls);
                    if (cls->parent.empty()) break;
                    cls = map.at(cls->parent);
                }
                for (auto it = sorted.rbegin(); it != sorted.rend(); it++) {
                    typeAdvisor.AddType(*it);
                }
            }
        }
    };

    auto stable = ctx.Get<ScopedTableSpecializer<SymbolTable>>("symbol_table");
    Visitor vis(*stable);
    vis.Visit(prog);
    ctx.Set<TypeAdvisor>("type_advisor", vis.typeAdvisor);
    return prog;
}

void ana::BuildInheritanceTree::Required() { pass::PassManager::Required<BuildInheritanceTree, InitSymbolTable>(); }

repr::Program ana::TypeChecking::operator()(repr::Program& prog, pass::PassContext& ctx) {
    using namespace visitor;

    using TypeName = string;

    class Visitor : public ProgramVisitor, ClassVisitor, FuncFeatureVisitor,
        FieldFeatureVisitor, FormalVisitor, ExprVisitor<TypeName> {

      public:
        ScopedTableSpecializer<SymbolTable>& stable;
        TypeAdvisor& typeAdvisor;

        Visitor(ScopedTableSpecializer<SymbolTable>& _stable, TypeAdvisor& _typeAdvisor)
        : stable(_stable), typeAdvisor(_typeAdvisor) {
            stable.InitTraverse();
        }

        void Visit(repr::Program &prog) {
            ENTER_SCOPE_GUARD(stable,for (auto& cls : prog.classes) Visit(*cls);)
        }

        void Visit(repr::Class &cls) {
            ENTER_SCOPE_GUARD(stable, {
                for (auto& feat : cls.fields) Visit(*feat);
                for (auto& feat : cls.funcs) Visit(*feat);
            })
        }

        void Visit(repr::FieldFeature &feat) {
            if (!typeAdvisor.Conforms(ExprVisitor<TypeName>::Visit(*feat.expr), feat.type)) {
                throw runtime_error("invalid initialization");
            }
        }

        void Visit(repr::FuncFeature &feat) {
            ENTER_SCOPE_GUARD(stable, {
                if (!typeAdvisor.Conforms(ExprVisitor<TypeName>::Visit(*feat.expr), feat.type)) {
                    throw runtime_error("invalid return type");
                }
            })
        }

        void Visit(repr::Formal &form) {}

        TypeName Visit_(repr::Assign& expr) {
            auto typeName = ExprVisitor<TypeName>::Visit(*expr.expr);
            if (!typeAdvisor.Conforms(typeName, stable.GetIdAttr(expr.id->name)->type)) {
                throw runtime_error("invalid assignment");
            }
            return typeName;
        }

        TypeName Visit_(repr::Add& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.left) != "Int" ||
                ExprVisitor<TypeName>::Visit(*expr.right) != "Int")
                throw runtime_error("operand for + must be Int");
            return "Int";
        }

        TypeName Visit_(repr::Block& expr) {
            ENTER_SCOPE_GUARD(stable, {
                for (int i = 0; i < expr.exprs.size() - 1; i++) ExprVisitor<TypeName>::Visit(*expr.exprs.at(i));
                return ExprVisitor<TypeName>::Visit(*expr.exprs.back());
            })
        }

        // todo: check duplicate types
        TypeName Visit_(repr::Case& expr) {
            vector<string> types;
            for (auto& branch : expr.branches) {
                ENTER_SCOPE_GUARD(stable, {
                    types.emplace_back(ExprVisitor<TypeName>::Visit(*branch->expr));
                })
            }
            return typeAdvisor.LeastCommonAncestor(types);
        }

        TypeName Visit_(repr::Call& expr) {
            ENTER_SCOPE_GUARD(stable, return CheckCall(stable.GetClass()->name, expr);)
        }

        TypeName Visit_(repr::MethodCall& expr) {
            ENTER_SCOPE_GUARD(stable,
                return CheckCall(ExprVisitor<TypeName>::Visit(*expr.left), *static_pointer_cast<repr::Call>(expr.right));)
        }

        TypeName CheckCall(TypeName type, repr::Call& expr) {
            TypeName rtype = "";
            auto dispatch = [&](shared_ptr<repr::Class> cls) {
                auto it = find_if(cls->funcs.begin(), cls->funcs.end(),
                                  [&expr](shared_ptr<repr::FuncFeature>& func){ return func->name == expr.id->name; });
                if (it == cls->funcs.end()) return false;
                if (expr.args.size() != (*it)->args.size()) throw runtime_error("dispatch failed");
                for (int i = 0; i < expr.args.size(); i++) {
                    if (!typeAdvisor.Conforms(ExprVisitor<TypeName>::Visit(*expr.args.at(i)), cls->funcs.at(i)->name)) {
                        throw runtime_error("dispatch failed");
                    }
                }
                rtype = (*it)->type;
            };
            typeAdvisor.BottomUpVisit(type, dispatch);
            return rtype;
        }

        TypeName Visit_(repr::Divide& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.left) != "Int" ||
                ExprVisitor<TypeName>::Visit(*expr.right) != "Int")
                throw runtime_error("operand for / must be Int");
            return "Int";
        }

        TypeName Visit_(repr::Equal& expr) {
            auto leftType = ExprVisitor<TypeName>::Visit(*expr.left);
            if (leftType == "Int" || leftType == "String" || leftType == "Bool") {
                if (leftType != ExprVisitor<TypeName>::Visit(*expr.right)) {
                    throw runtime_error("invalid operand types for =");
                }
            }
            return "Bool";
        }

        TypeName Visit_(repr::False& expr) {
            return "Bool";
        }

        TypeName Visit_(repr::ID& expr) {
            return stable.GetIdAttr(expr.name)->type;
        }

        TypeName Visit_(repr::IsVoid& expr) {
            return "Bool";
        }

        TypeName Visit_(repr::Integer& expr) {
            return "Int";
        }

        TypeName Visit_(repr::If& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.ifExpr) != "Bool") {
                throw runtime_error("if predicate must be Bool");
            }
            return typeAdvisor.LeastCommonAncestor(
                ExprVisitor<TypeName>::Visit(*expr.thenExpr),
                ExprVisitor<TypeName>::Visit(*expr.elseExpr));
        }

        TypeName Visit_(repr::LessThanOrEqual& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.left) != "Int" ||
                ExprVisitor<TypeName>::Visit(*expr.right) != "Int")
                throw runtime_error("operand for <= must be Int");
            return "Bool";
        }

        TypeName Visit_(repr::LessThan& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.left) != "Int" ||
                ExprVisitor<TypeName>::Visit(*expr.right) != "Int")
                throw runtime_error("operand for < must be Int");
            return "Bool";
        }

        TypeName Visit_(repr::Let& expr) {
            for (int i = 0; i < expr.formals.size(); i++) {
                ENTER_SCOPE_GUARD(stable, {
                    auto& form = expr.formals.at(i);
                    if (form->expr && !typeAdvisor.Conforms(ExprVisitor<TypeName>::Visit(*form->expr), form->type)) {
                        throw runtime_error("invalid initialization");
                    }
                    if (i == expr.formals.size()-1) {
                        return ExprVisitor<TypeName>::Visit(*expr.expr);
                    }
                })
            }
        }

        TypeName Visit_(repr::Multiply& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.left) != "Int" ||
                ExprVisitor<TypeName>::Visit(*expr.right) != "Int")
                throw runtime_error("operand for * must be Int");
            return "Int";
        }

        TypeName Visit_(repr::Minus& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.left) != "Int" ||
                ExprVisitor<TypeName>::Visit(*expr.right) != "Int")
                throw runtime_error("operand for - must be Int");
            return "Int";
        }

        TypeName Visit_(repr::Negate& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.expr) != "Int")
                throw runtime_error("operand for ~ must be Int");
            return "Int";
        }

        TypeName Visit_(repr::New& expr) {
            return expr.type;
        }

        TypeName Visit_(repr::Not& expr) {
            if (ExprVisitor<TypeName>::Visit(*expr.expr) != "Bool") {
                throw runtime_error("operand for not must be Bool");
            }
            return "Bool";
        }

        TypeName Visit_(repr::String& expr) {
            return "String";
        }

        TypeName Visit_(repr::True& expr) {
            return "Bool";
        }

        TypeName Visit_(repr::While& expr) {
            ENTER_SCOPE_GUARD(stable, {
                if (ExprVisitor<TypeName>::Visit(*expr.whileExpr) != "Bool") {
                    throw runtime_error("the type of while predicate must be Bool");
                }
                return "Object";
            })
        }
    };

    auto stable = ctx.Get<ScopedTableSpecializer<SymbolTable>>("symbol_table");
    auto typeAdvisor = ctx.Get<TypeAdvisor>("type_advisor");
    Visitor vis(*stable, *typeAdvisor);
    vis.Visit(prog);
    return prog;
}

void ana::TypeChecking::Required() { pass::PassManager::Required<TypeChecking, BuildInheritanceTree>(); }
