//
// Created by 田地 on 2021/6/19.
//

#ifndef COOL_ANALYSIS_H
#define COOL_ANALYSIS_H

#include "pass.h"

namespace cool {

namespace ana {

class InstallBuiltin : public pass::Pass {
  public:
    repr::Program operator()(repr::Program& prog, pass::PassContext& ctx) final;
};

class InitSymbolTable : public pass::Pass {
  public:
    repr::Program operator()(repr::Program& prog, pass::PassContext& ctx) final;
};

class BuildInheritanceTree : public pass::Pass {
  public:
    repr::Program operator()(repr::Program& prog, pass::PassContext& ctx) final;
};

class TypeChecking : public pass::Pass {
  public:
    repr::Program operator()(repr::Program& prog, pass::PassContext& ctx) final;
};

} // ana

} // cool

#endif //COOL_ANALYSIS_H
