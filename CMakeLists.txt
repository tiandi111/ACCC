cmake_minimum_required(VERSION 3.16)
project(cool)

set(CMAKE_CXX_STANDARD 14)
set(ENV{LLVM_DIR} /usr/local/Cellar/llvm/12.0.1/lib/cmake)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(cool

        frontend/parser.h frontend/parser.cpp
        frontend/repr.h frontend/repr.cpp
        frontend/token.h frontend/token.cpp
        frontend/pass.h frontend/pass.cpp
        frontend/tokenizer.h frontend/tokenizer.cpp
        frontend/analysis.h frontend/analysis.cpp
        frontend/adt.h
        frontend/attrs.h
        frontend/visitor.h
        frontend/vtable.h
        frontend/builtin.h frontend/builtin.cpp
        frontend/typead.h frontend/typead.cpp
        frontend/diag.h frontend/diag.cpp
        frontend/llvm_gen.h frontend/llvm_gen.cpp

#        middleend/mini-llvm/core.h
#        middleend/mini-llvm/ir/bb.h
#        middleend/mini-llvm/ir/builder.h
#        middleend/mini-llvm/ir/ctx.h
#        middleend/mini-llvm/ir/inst.h
#        middleend/mini-llvm/ir/module.h
#        middleend/mini-llvm/ir/type.h
#        middleend/mini-llvm/ir/value.h

        main.cpp)

add_executable(utest

        frontend/parser.h frontend/parser.cpp
        frontend/repr.h frontend/repr.cpp
        frontend/token.h frontend/token.cpp
        frontend/pass.h frontend/pass.cpp
        frontend/tokenizer.h frontend/tokenizer.cpp
        frontend/analysis.h frontend/analysis.cpp
        frontend/adt.h
        frontend/attrs.h
        frontend/visitor.h
        frontend/vtable.h
        frontend/builtin.h frontend/builtin.cpp
        frontend/typead.h frontend/typead.cpp
        frontend/diag.h frontend/diag.cpp
        frontend/llvm_gen.h frontend/llvm_gen.cpp

#        middleend/mini-llvm/core.h
#        middleend/mini-llvm/ir/bb.h
#        middleend/mini-llvm/ir/builder.h
#        middleend/mini-llvm/ir/ctx.h
#        middleend/mini-llvm/ir/inst.h
#        middleend/mini-llvm/ir/module.h
#        middleend/mini-llvm/ir/type.h
#        middleend/mini-llvm/ir/value.h

        test/unit.h test/unit.cpp)

add_executable(itest

        frontend/parser.h frontend/parser.cpp
        frontend/repr.h frontend/repr.cpp
        frontend/token.h frontend/token.cpp
        frontend/pass.h frontend/pass.cpp
        frontend/tokenizer.h frontend/tokenizer.cpp
        frontend/analysis.h frontend/analysis.cpp
        frontend/adt.h
        frontend/attrs.h
        frontend/visitor.h
        frontend/vtable.h
        frontend/builtin.h frontend/builtin.cpp
        frontend/typead.h frontend/typead.cpp
        frontend/diag.h frontend/diag.cpp
        frontend/llvm_gen.h frontend/llvm_gen.cpp

#        middleend/mini-llvm/core.h
#        middleend/mini-llvm/ir/bb.h
#        middleend/mini-llvm/ir/builder.h
#        middleend/mini-llvm/ir/ctx.h
#        middleend/mini-llvm/ir/inst.h
#        middleend/mini-llvm/ir/module.h
#        middleend/mini-llvm/ir/type.h
#        middleend/mini-llvm/ir/value.h

        test/integration/semantics.h test/integration/semantics.cpp)

add_executable(runtime
        runtime/runtime.h runtime/runtime.c)

llvm_map_components_to_libnames(llvm_libs support core x86asmparser x86codegen x86desc x86disassembler x86info)

target_link_libraries(cool ${llvm_libs})
target_link_libraries(utest ${llvm_libs})
target_link_libraries(itest ${llvm_libs})
target_link_libraries(runtime ${llvm_libs})
