ACCC (A C++ Cool Compiler)
---

### Project Description
ACCC is a C++ compiler for Cool (the Classroom Object-Oriented Language). The main purpose of this project is to
familiarize myself with compiler architecture, principles and techniques. Hence, this compiler is implemented without
any extra compiler tools.

### About Cool
Cool is a small object-oriented language designed by Stanford University. It is supposed to be implemented with reasonable
effort in a one semester course while still retains many of the features of modern programming languages
including objects, static typing, and automatic memory management. The language specification can be found here:
[cool-manual](http://theory.stanford.edu/~aiken/software/cool/cool-manual.pdf).

### Installation & Use
TODO

### Development Status
| Compiler Stage          |        Status       |
| ------------------------| ------------------- |
| Lexical Analysis        |      ✅ done        |
| Syntactic Analysis      |      ✅ done        |
| Semantic Analysis       |      ✅ done        |
| Runtime System          |    ⭕️ in progress   |
| LLVM IR Generation      |    ⭕️ in progress   |
| Mini-LLVM Infra         |        -            |
| Optimization            |        -            |
| Machine Code Generation |        -            |
| Garbage Collection      |        -            | 

### Architecture
TODO

### Code Organization
- Compiler Stage
    - Lexical Analysis: token.h / token.cpp / tokenizer.h / tokenizer.cpp
    - Syntactic Analysis: parser.h / parser.cpp
    - Semantic Analysis: analysis.h / analysis.cpp
- Infrastructure
    - Abstract Syntax Tree: repr.h / repr.cpp
    - Symbol Table & Inheritance Tree: attrs.g / stable.h / typead.h / typead.cpp
    - Visitor: visitor.h / vtable.h
    - Pass Management: pass.h / pass.cpp
    - Diagnosis Management: diag.h / diag.cpp
    - Built-in Support: builtin.h / builtin.cpp
- Test: ./test

### LITERATURE
- Engineering a Compiler, Cooper and Torczon
- Compiler: Principles, Techniques and Tools, Alfred V.Aho, Monica S.Lam, Ravi Sethi and Jeffery D.Ullman
- Modern Compiler Implementation in Java, Andrew W., Appel and Jens Palsberg
- Introduction to Compilers and Language Design, Douglas Thain
- Stanford's Compiler Theory Course (CS143, CS243)

### Other Cool Compilers
- [PyCOOLC](https://github.com/aalhour/PyCOOLC)

### Author
Di Tian


