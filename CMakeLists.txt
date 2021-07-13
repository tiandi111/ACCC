cmake_minimum_required(VERSION 3.16)
project(cool)

set(CMAKE_CXX_STANDARD 14)

add_executable(cool
        parser.h parser.cpp
        repr.h repr.cpp
        token.h token.cpp
        pass.h pass.cpp
        tokenizer.h tokenizer.cpp
        analysis.h analysis.cpp
        stable.h
        attrs.h
        visitor.h
        vtable.h
        builtin.h builtin.cpp
        typead.h typead.cpp
        diag.h diag.cpp
        main.cpp)

add_executable(UTest
        parser.h parser.cpp
        repr.h repr.cpp
        token.h token.cpp
        pass.h pass.cpp
        tokenizer.h tokenizer.cpp
        analysis.h analysis.cpp
        stable.h
        attrs.h
        visitor.h
        vtable.h
        builtin.h builtin.cpp
        typead.h typead.cpp
        diag.h diag.cpp
        test/unit.h test/unit.cpp)

add_executable(ITest
        parser.h parser.cpp
        repr.h repr.cpp
        token.h token.cpp
        pass.h pass.cpp
        tokenizer.h tokenizer.cpp
        analysis.h analysis.cpp
        stable.h
        attrs.h
        visitor.h
        vtable.h
        builtin.h builtin.cpp
        typead.h typead.cpp
        diag.h diag.cpp
        test/integration/semantics.h test/integration/semantics.cpp)