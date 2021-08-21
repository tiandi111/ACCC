//
// Created by 田地 on 2021/6/3.
//

#ifndef COOL_TOKENIZER_H
#define COOL_TOKENIZER_H

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <istream>

#include "token.h"
#include "diag.h"

using namespace std;

namespace cool {

namespace tok {

extern unordered_map<string, Token::Type> keywordMap;
extern unordered_map<char, Token::Type> tokenMap;
extern unordered_map<Token::Type, string> tokenStr;

class Tokenizer {
  private:
    int line;
    int pos;
    int fileno;
    diag::Diagnosis& diag;

  public:
    Tokenizer(diag::Diagnosis& _diag);

    vector<Token> Tokenize(const string& file, istream& in);

    // todo: support comments
    Token TokDigit(istream& in);
    Token TokAlpha(istream& in);
    Token TokString(istream& in);
    Token TokSpecial(istream& in);
    Token TokComment(istream& in);
};

} // namespace cool

} // namespace tok

#endif //COOL_TOKENIZER_H
