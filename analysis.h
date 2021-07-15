//
// Created by 田地 on 2021/6/19.
//

#ifndef COOL_ANALYSIS_H
#define COOL_ANALYSIS_H

#include "pass.h"

namespace cool {

namespace ana {

// note (2021-07-14 16:21:38): I found that finer-grained passes are easier to manage
// e.g, it is easier to reorder them since each pass has less dependencies.
// However, coarse-grained passes are more modular, e.g some passes may depend on other
// specific pre-checking passes, making them as one pass is self-contained.
// I use finer-grained passes and group related passes into containers, e.g Sequential,
// to get the benefits of both.

class InstallBuiltin : public pass::ProgramPass {
  public:
    repr::Program operator()(repr::Program& prog, pass::PassContext& ctx) final;
};

class CheckBuiltinInheritance : public pass::ClassPass {
  public:
    repr::Class operator()(repr::Class& cls, pass::PassContext& ctx) final;
};

class BuildInheritanceTree : public pass::ProgramPass {
  public:
    repr::Program operator()(repr::Program& prog, pass::PassContext& ctx) final;
};

class CheckInheritedAttributes : public pass::ClassPass {
  public:
    repr::Class operator()(repr::Class& cls, pass::PassContext& ctx) final;
};

class AddInheritedAttributes : public pass::ClassPass {
  public:
    repr::Class operator()(repr::Class& cls, pass::PassContext& ctx) final;
};

class CheckInheritedMethods : public pass::ClassPass {
public:
    repr::Class operator()(repr::Class& cls, pass::PassContext& ctx) final;
};

class InitSymbolTable : public pass::ProgramPass {
  public:
    repr::Program operator()(repr::Program& prog, pass::PassContext& ctx) final;
};

class TypeChecking : public pass::ProgramPass {
  public:
    repr::Program operator()(repr::Program& prog, pass::PassContext& ctx) final;
};

class SemanticChecking : public pass::Sequential {
  public:
    SemanticChecking() : pass::Sequential({
        make_shared<InstallBuiltin>(InstallBuiltin()),
        make_shared<CheckBuiltinInheritance>(CheckBuiltinInheritance()),
        make_shared<BuildInheritanceTree>(BuildInheritanceTree()),
        make_shared<CheckInheritedAttributes>(CheckInheritedAttributes()),
        make_shared<AddInheritedAttributes>(AddInheritedAttributes()),
        make_shared<CheckInheritedMethods>(CheckInheritedMethods()),
        make_shared<InitSymbolTable>(InitSymbolTable()),
        make_shared<TypeChecking>(TypeChecking())
    }) {}
};

} // ana

} // cool

#endif //COOL_ANALYSIS_H
