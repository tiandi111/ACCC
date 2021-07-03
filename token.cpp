//
// Created by 田地 on 2021/7/3.
//

#include "token.h"
#include "diag.h"

using namespace cool;
using namespace tok;

Token::Token(Type _type, string _str, string _val, int _line, int _pos, int _fileno)
: type(_type), str(std::move(_str)), val(std::move(_val)), textInfo(diag::TextInfo{_line, _pos, _fileno}) {
    assert(type > ST && type < END);
}

bool Token::Skip() { return type == SKIP; }