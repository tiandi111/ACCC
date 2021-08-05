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

// todo: check Main class, main function
// todo: replace SELF_TYPE with real type？

class InstallBuiltin : public pass::ProgramPass {
  public:
    repr::Program* operator()(repr::Program* prog, pass::PassContext& ctx) final;
};

class CheckBuiltinInheritance : public pass::ClassPass {
  public:
    repr::Class* operator()(repr::Class* cls, pass::PassContext& ctx) final;
};

class BuildInheritanceTree : public pass::ProgramPass {
  public:
    repr::Program* operator()(repr::Program* prog, pass::PassContext& ctx) final;
};

class CheckInheritedAttributes : public pass::ClassPass {
  public:
    repr::Class* operator()(repr::Class* cls, pass::PassContext& ctx) final;
};

class AddInheritedAttributes : public pass::ClassPass {
  public:
    repr::Class* operator()(repr::Class* cls, pass::PassContext& ctx) final;
};

class CheckInheritedMethods : public pass::ClassPass {
  public:
    repr::Class* operator()(repr::Class* cls, pass::PassContext& ctx) final;
};

class AddInheritedMethods : public pass::ClassPass {
  public:
    repr::Class* operator()(repr::Class* cls, pass::PassContext& ctx) final;
};

class InitSymbolTable : public pass::ProgramPass {
  public:
    repr::Program* operator()(repr::Program* prog, pass::PassContext& ctx) final;
};

class TypeChecking : public pass::ProgramPass {
  public:
    repr::Program* operator()(repr::Program* prog, pass::PassContext& ctx) final;
};

class EliminateSelfType : public pass::ProgramPass {
  public:
    repr::Program* operator()(repr::Program* prog, pass::PassContext& ctx) final;
};

class SemanticChecking : public pass::Sequential {
  public:
    SemanticChecking() : pass::Sequential({
        make_shared<InstallBuiltin>(),
        make_shared<CheckBuiltinInheritance>(),
        make_shared<BuildInheritanceTree>(),
        make_shared<CheckInheritedAttributes>(),
        make_shared<AddInheritedAttributes>(),
        make_shared<CheckInheritedMethods>(),
        make_shared<AddInheritedMethods>(),
        make_shared<InitSymbolTable>(),
        make_shared<TypeChecking>(),
        make_shared<EliminateSelfType>()
    }) {}
};

} // ana

} // cool

#endif //COOL_ANALYSIS_H
