#pragma once
#include <string>
#include "token.h"

// TOKENISER / LEXER
/*
FOF = first order form
CNF = clause normal form
END = end of file "sentinel token"
(note that WORD could be 'fof', 'conjecture', 'p', 'john' etc.)
*/
class Tokeniser {
public:
    Tokeniser(const std::string& input) : input(input), pos(0) {}
    Token nextToken();

private:
    std::string input;
    size_t pos;
    void skipWhitespaceAndComments();
};