//
// Created by 田地 on 2021/5/30.
//

#ifndef COOL_TOKEN_H
#define COOL_TOKEN_H

#include <string>
#include <utility>
#include <unordered_map>

using namespace std;

namespace cool {

namespace tok {

struct Token {
    enum Type {
        ST,
        ID,
        TypeID,
        Integer,
        String,
        kTrue,
        kFalse,
        kIf,
        kThen,
        kElse,
        kFi,
        kWhile,
        kLoop,
        kPool,
        kLet,
        kIn,
        kCase,
        kOf,
        kEsac,
        kNew,
        kIsvoid,
        kNot,
        kBinaryST,
        kAdd,
        kMinus,
        kMultiply,
        kDivide,
        kLessThan,
        kLessThanOrEqual,
        kEqual,
        kDot,
        kBinaryEND,
        kNegate,
        kClass,
        kInheirits,
        kOpenParen,
        kCloseParen,
        kSemiColon,
        kColon,
        kComma,
        kOpenBrace,
        kCloseBrace,
        kAssignment,
        kEval,
        SKIP, // skipped token
        END
    };
    Type type;
    string str;
    string val;
    int line = 0;
    int pos = 0;
    int fileno = 0;

    Token(Type _type, string _str, string _val, int _line, int _pos, int _fileno = -1)
        : type(_type), str(std::move(_str)), val(std::move(_val)), line(_line), pos(_pos), fileno(_fileno) {
        assert(type > ST && type < END);
    }

    bool Skip() { return type == SKIP; }
};

class FileMapper {
  private:
    unordered_map<string, int> name2no;
    unordered_map<int, string> no2name;

public:
    static FileMapper& GetFileMapper() {
        static FileMapper fileMapper;
        return fileMapper;
    }

    int GetFileNo(const string& fname) {
        if (name2no.find(fname) == name2no.end()) {
            name2no.insert({fname, name2no.size()});
            no2name.insert({no2name.size(), fname});
        }
        return name2no.at(fname);
    }

    string GetFileName(int fileno) {
        if (fileno == -1) return "";
        if (no2name.find(fileno) == no2name.end()) throw runtime_error("invalid file number");
        return no2name.at(fileno);
    }
};

} // namespace tok

} // namespace cool


#endif //COOL_TOKEN_H
