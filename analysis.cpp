//
// Created by 田地 on 2021/6/14.
//

#include "analysis.h"
#include "builtin.h"
#include "pass.h"
#include "stable.h"
#include "visitor.h"

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
            if (cls.parent.empty()) {
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
        ScopedTable<SymbolTable> stable;

        void Visit(repr::Program &prog) {
            NEW_SCOPE_GUARD(stable,{
                for (auto &cls : prog.classes) { Visit(*cls); }
            })
        }

        void Visit(repr::Class &cls) {
            stable.Current().Insert(attr::TypeAttr{cls.name});
            NEW_SCOPE_GUARD(stable, {
                for (auto& feat : cls.funcs) { Visit(*feat); }
                for (auto& feat : cls.fields) { Visit(*feat); }
            })
        }

        void Visit(repr::FuncFeature &feat) {
            stable.Current().Insert(attr::FuncAttr{feat.name, feat.type});
            NEW_SCOPE_GUARD(stable, {
                for (auto &form : feat.args) { Visit(*form); }
                if (feat.expr) ExprVisitor::Visit(*feat.expr);
            })
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
            })
        }
        void Visit_(repr::Case& expr) {
            ExprVisitor::Visit(*expr.expr);
            for (auto& branch : expr.branches) {
                NEW_SCOPE_GUARD(stable, {
                    stable.Current().Insert(attr::IdAttr{branch->id, branch->type});
                    ExprVisitor::Visit(*branch->expr);
                })
            }
        }
        void Visit_(repr::Call& expr) {
            NEW_SCOPE_GUARD(stable, {
                Visit_(*expr.id);
                for (auto &arg : expr.args) { ExprVisitor::Visit(*arg); }
            })
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
                NEW_SCOPE_GUARD(stable, *expr.thenExpr);
                NEW_SCOPE_GUARD(stable, *expr.elseExpr);
            })
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
                })
            }
        }
        void Visit_(repr::MethodCall& expr) {
            NEW_SCOPE_GUARD(stable, {
                ExprVisitor::Visit(*expr.left);
                ExprVisitor::Visit(*expr.right);
            })
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
            })
        }

        void VisitBinary(repr::Binary& expr) {
            ExprVisitor::Visit(*expr.left);
            ExprVisitor::Visit(*expr.right);
        }
    };

    Visitor vis;
    vis.Visit(prog);
    ctx.Set<ScopedTable<SymbolTable>>("symbol_table", vis.stable);
    return prog;
}

repr::Program ana::BuildInheritanceTree::operator()(repr::Program& prog, pass::PassContext& ctx) {
    class Visitor : public visitor::ProgramVisitor, visitor::ClassVisitor {
      public:
        ScopedTable<SymbolTable>& stable;

        Visitor(ScopedTable<SymbolTable>& _stable) : stable(_stable) {
            stable.InitTraverse();
        }

        void Visit(repr::Program& prog) {
            ENTER_SCOPE_GUARD(stable, {
                for (auto& cls : prog.classes) {
                    Visit(*cls);
                }
            })
        }

        void Visit(repr::Class &cls) {
            auto& symbolTable = stable.Current();
            auto typeAttr = symbolTable.GetTypeAttr(cls.name);
            auto parentTypeAttr = symbolTable.GetTypeAttr(cls.parent);
            typeAttr->parent = parentTypeAttr;
            parentTypeAttr->children.push_back(typeAttr);
        }
    };

    auto stable = ctx.Get<ScopedTable<SymbolTable>>("symbol_table");
    Visitor vis(*stable);
    vis.Visit(prog);
    return prog;
}

repr::Program ana::TypeChecking::operator()(repr::Program& prog, pass::PassContext& ctx) {
    using namespace visitor;

    class Visitor : public ProgramVisitor, ClassVisitor, FuncFeatureVisitor,
        FieldFeatureVisitor, FormalVisitor, ExprVisitor<attr::TypeAttr> {

      public:
        ScopedTable<SymbolTable>& stable;

        Visitor(ScopedTable<SymbolTable>& _stable) : stable(_stable) {
            stable.InitTraverse();
        }

        void Visit(repr::Program &prog) {
            ENTER_SCOPE_GUARD(stable, {
                for (auto& cls : prog.classes) Visit(*cls);
            })
        }

        void Visit(repr::Class &cls) {
            ENTER_SCOPE_GUARD(stable, {
                for (auto& feat : cls.fields) Visit(*feat);
                for (auto& feat : cls.funcs) Visit(*feat);
            })
        }

        void Visit(repr::FieldFeature &feat) {
            if (!ExprVisitor<attr::TypeAttr>::Visit(*feat.expr).Conforms(feat.type)) {
                throw runtime_error("invalid initialization");
            }
        }

        void Visit(repr::FuncFeature &feat) {
            ENTER_SCOPE_GUARD(stable, {
                if (!ExprVisitor<attr::TypeAttr>::Visit(*feat.expr).Conforms(feat.type)) {
                    throw runtime_error("invalid return type");
                }
            })
        }

        void Visit(repr::Formal &form) { return; }

        attr::TypeAttr Visit_(repr::Assign& expr) {
            auto typeAttr = ExprVisitor<attr::TypeAttr>::Visit(*expr.expr);
            if (!typeAttr.Conforms(stable.Current().GetIdAttr(expr.id->name)->type)) {
                throw runtime_error("invalid assignment");
            }
            return typeAttr;
        }

        attr::TypeAttr Visit_(repr::Add& expr) {
            if (!ExprVisitor<attr::TypeAttr>::Visit(*expr.left).Equal("Int") ||
                !ExprVisitor<attr::TypeAttr>::Visit(*expr.right).Equal("Int"))
                throw runtime_error("operand for + must be Int");
        }

        attr::TypeAttr Visit_(repr::Block& expr) {
            ENTER_SCOPE_GUARD(stable, {
                for (auto &x : expr.exprs) ExprVisitor<attr::TypeAttr>::Visit(*x);
                return ExprVisitor<attr::TypeAttr>::Visit(*expr.exprs.back());
            })
        }

        attr::TypeAttr Visit_(repr::Case& expr) {
            vector<shared_ptr<attr::TypeAttr>> attrs;
            for (auto& branch : expr.branches) {
                ENTER_SCOPE_GUARD(stable, {
                    attrs.emplace_back(make_shared<attr::TypeAttr>(ExprVisitor<attr::TypeAttr>::Visit(*branch->expr)));
                })
            }
            return *attr::TypeAttr::LeastCommonAncestor(attrs);
        }

        attr::TypeAttr Visit_(repr::Call& expr) {

        }

        attr::TypeAttr Visit_(repr::Divide& expr) {
            if (!ExprVisitor<attr::TypeAttr>::Visit(*expr.left).Equal("Int") ||
                !ExprVisitor<attr::TypeAttr>::Visit(*expr.right).Equal("Int"))
                throw runtime_error("operand for / must be Int");
            return *stable.Current().GetTypeAttr("Int");
        }

        attr::TypeAttr Visit_(repr::Equal& expr) {
            auto leftType = ExprVisitor<attr::TypeAttr>::Visit(*expr.left);
            if (leftType.Equal("Int") || leftType.Equal("String") || leftType.Equal("Bool")) {
                if (!leftType.Equal(ExprVisitor<attr::TypeAttr>::Visit(*expr.right))) {
                    throw runtime_error("invalid operand types for =");
                }
            }
            return *stable.Current().GetTypeAttr("Bool");
        }

        attr::TypeAttr Visit_(repr::False& expr) {
            return *stable.Current().GetTypeAttr("Bool");
        }

        attr::TypeAttr Visit_(repr::ID& expr) {
            auto& cur = stable.Current();
            return *cur.GetTypeAttr(cur.GetIdAttr(expr.name)->type);
        }

        attr::TypeAttr Visit_(repr::IsVoid& expr) {
            return *stable.Current().GetTypeAttr("Bool");
        }

        attr::TypeAttr Visit_(repr::Integer& expr) {
            return *stable.Current().GetTypeAttr("Int");
        }

        attr::TypeAttr Visit_(repr::If& expr) {}

        attr::TypeAttr Visit_(repr::LessThanOrEqual& expr) {
            if (!ExprVisitor<attr::TypeAttr>::Visit(*expr.left).Equal("Int") ||
                !ExprVisitor<attr::TypeAttr>::Visit(*expr.right).Equal("Int"))
                throw runtime_error("operand for <= must be Int");
            return *stable.Current().GetTypeAttr("Bool");
        }

        attr::TypeAttr Visit_(repr::LessThan& expr) {
            if (!ExprVisitor<attr::TypeAttr>::Visit(*expr.left).Equal("Int") ||
                !ExprVisitor<attr::TypeAttr>::Visit(*expr.right).Equal("Int"))
                throw runtime_error("operand for < must be Int");
            return *stable.Current().GetTypeAttr("Bool");
        }

        attr::TypeAttr Visit_(repr::Let& expr) {
            for (int i = 0; i < expr.formals.size(); i++) {
                ENTER_SCOPE_GUARD(stable, {
                    auto& form = expr.formals.at(i);
                    if (form->expr && !ExprVisitor<attr::TypeAttr>::Visit(*form->expr).Conforms(form->type)) {
                        throw runtime_error("invalid initialization");
                    }
                    if (i == expr.formals.size()-1) {
                        return ExprVisitor<attr::TypeAttr>::Visit(*expr.expr);
                    }
                })
            }
        }

        attr::TypeAttr Visit_(repr::MethodCall& expr) {

        }

        attr::TypeAttr Visit_(repr::Multiply& expr) {
            if (!ExprVisitor<attr::TypeAttr>::Visit(*expr.left).Equal("Int") ||
                !ExprVisitor<attr::TypeAttr>::Visit(*expr.right).Equal("Int"))
                throw runtime_error("operand for * must be Int");
            return *stable.Current().GetTypeAttr("Int");
        }

        attr::TypeAttr Visit_(repr::Minus& expr) {
            if (!ExprVisitor<attr::TypeAttr>::Visit(*expr.left).Equal("Int") ||
                !ExprVisitor<attr::TypeAttr>::Visit(*expr.right).Equal("Int"))
                throw runtime_error("operand for - must be Int");
            return *stable.Current().GetTypeAttr("Int");
        }

        attr::TypeAttr Visit_(repr::Negate& expr) {
            if (!ExprVisitor<attr::TypeAttr>::Visit(*expr.expr).Equal("Int"))
                throw runtime_error("operand for ~ must be Int");
            return *stable.Current().GetTypeAttr("Int");
        }

        attr::TypeAttr Visit_(repr::New& expr) {
            return *stable.Current().GetTypeAttr(expr.type);
        }

        attr::TypeAttr Visit_(repr::Not& expr) {
            if (!ExprVisitor<attr::TypeAttr>::Visit(*expr.expr).Equal("Bool")) {
                throw runtime_error("operand for not must be Bool");
            }
            return *stable.Current().GetTypeAttr("Bool");
        }

        attr::TypeAttr Visit_(repr::String& expr) {
            return *stable.Current().GetTypeAttr("String");
        }

        attr::TypeAttr Visit_(repr::True& expr) {
            return *stable.Current().GetTypeAttr("Bool");
        }

        attr::TypeAttr Visit_(repr::While& expr) {
            ENTER_SCOPE_GUARD(stable, {
                if (!ExprVisitor<attr::TypeAttr>::Visit(*expr.whileExpr).Equal("Bool")) {
                    throw runtime_error("the type of while predicate must be Bool");
                }
                return *stable.Current().GetTypeAttr("Object");
            })
        }
    };

    auto stable = ctx.Get<ScopedTable<SymbolTable>>("symbol_table");
    Visitor vis(*stable);
    vis.Visit(prog);
    return prog;
}
