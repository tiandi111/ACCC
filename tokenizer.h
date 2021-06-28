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

using namespace std;

extern unordered_map<string, Token> keywordMap;
extern unordered_map<char, Token> tokenMap;

class Tokenizer {
  private:
//    vector<istream> ins;

  public:
     Tokenizer() {}

     vector<Token> Tokenize(istream& in);

     Token TokDigit(istream& in);
     Token TokAlpha(istream& in);
     Token TokString(istream& in);
     Token TokSpecial(istream& in);
};

#endif //COOL_TOKENIZER_H
