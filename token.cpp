//
// Created by 田地 on 2021/7/3.
//

#include "token.h"

using namespace cool;
using namespace tok;

Token::Token(Type _type, string _str, string _val, int _line, int _pos, int _fileno)
: type(_type), str(std::move(_str)), val(std::move(_val)), line(_line), pos(_pos), fileno(_fileno) {
    assert(type > ST && type < END);
}

bool Token::Skip() { return type == SKIP; }

FileMapper& FileMapper::GetFileMapper() {
    static FileMapper fileMapper;
    return fileMapper;
}

int FileMapper::GetFileNo(const string& fname) {
    if (name2no.find(fname) == name2no.end()) {
        name2no.insert({fname, name2no.size()});
        no2name.insert({no2name.size(), fname});
    }
    return name2no.at(fname);
}

string FileMapper::GetFileName(int fileno) {
    if (fileno == -1) return "";
    if (no2name.find(fileno) == no2name.end()) throw runtime_error("invalid file number");
    return no2name.at(fileno);
}