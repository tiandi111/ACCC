cmake_minimum_required(VERSION 3.16)
project(cool)

set(CMAKE_CXX_STANDARD 14)

add_executable(cool
        parser.h parser.cpp
        repr.h
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
        diag.h
        main.cpp)

add_executable(unittest
        parser.h parser.cpp
        repr.h
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
        diag.h
        test/unit.h test/unit.cpp)