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

class Tokenizer {
  private:
    diag::Diagnosis& diag;

  public:
    Tokenizer(diag::Diagnosis& _diag);

    vector<Token> Tokenize(istream& in);

    Token TokDigit(int line, istream& in);
    Token TokAlpha(int line, istream& in);
    Token TokString(int line, istream& in);
    Token TokSpecial(int line, istream& in);
};

} // namespace cool

} // namespace tok

#endif //COOL_TOKENIZER_H
