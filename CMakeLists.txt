cmake_minimum_required(VERSION 3.16)
project(cool)

set(CMAKE_CXX_STANDARD 14)

add_executable(cool

        frontend/parser.h frontend/parser.cpp
        frontend/repr.h frontend/repr.cpp
        frontend/token.h frontend/token.cpp
        frontend/pass.h frontend/pass.cpp
        frontend/tokenizer.h frontend/tokenizer.cpp
        frontend/analysis.h frontend/analysis.cpp
        frontend/stable.h
        frontend/attrs.h
        frontend/visitor.h
        frontend/vtable.h
        frontend/builtin.h frontend/builtin.cpp
        frontend/typead.h frontend/typead.cpp
        frontend/diag.h frontend/diag.cpp

        middleend/mini-llvm/core.h
        middleend/mini-llvm/ir/bb.h
        middleend/mini-llvm/ir/builder.h
        middleend/mini-llvm/ir/ctx.h
        middleend/mini-llvm/ir/inst.h
        middleend/mini-llvm/ir/module.h
        middleend/mini-llvm/ir/type.h
        middleend/mini-llvm/ir/value.h

        main.cpp)

add_executable(UTest

        frontend/parser.h frontend/parser.cpp
        frontend/repr.h frontend/repr.cpp
        frontend/token.h frontend/token.cpp
        frontend/pass.h frontend/pass.cpp
        frontend/tokenizer.h frontend/tokenizer.cpp
        frontend/analysis.h frontend/analysis.cpp
        frontend/stable.h
        frontend/attrs.h
        frontend/visitor.h
        frontend/vtable.h
        frontend/builtin.h frontend/builtin.cpp
        frontend/typead.h frontend/typead.cpp
        frontend/diag.h frontend/diag.cpp

        middleend/mini-llvm/core.h
        middleend/mini-llvm/ir/bb.h
        middleend/mini-llvm/ir/builder.h
        middleend/mini-llvm/ir/ctx.h
        middleend/mini-llvm/ir/inst.h
        middleend/mini-llvm/ir/module.h
        middleend/mini-llvm/ir/type.h
        middleend/mini-llvm/ir/value.h

        test/unit.h test/unit.cpp)

add_executable(ITest
        frontend/parser.h frontend/parser.cpp
        frontend/repr.h frontend/repr.cpp
        frontend/token.h frontend/token.cpp
        frontend/pass.h frontend/pass.cpp
        frontend/tokenizer.h frontend/tokenizer.cpp
        frontend/analysis.h frontend/analysis.cpp
        frontend/stable.h
        frontend/attrs.h
        frontend/visitor.h
        frontend/vtable.h
        frontend/builtin.h frontend/builtin.cpp
        frontend/typead.h frontend/typead.cpp
        frontend/diag.h frontend/diag.cpp

        middleend/mini-llvm/core.h
        middleend/mini-llvm/ir/bb.h
        middleend/mini-llvm/ir/builder.h
        middleend/mini-llvm/ir/ctx.h
        middleend/mini-llvm/ir/inst.h
        middleend/mini-llvm/ir/module.h
        middleend/mini-llvm/ir/type.h
        middleend/mini-llvm/ir/value.h

        test/integration/semantics.h test/integration/semantics.cpp)