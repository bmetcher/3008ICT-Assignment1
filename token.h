#pragma once
#include <string>

enum class TokenType {
    FOF, CNF, END,
    LPARENTH, RPARENTH, LBRACKET, RBRACKET,
    COMMA, PERIOD, COLON, TILDE, AND, OR, 
    IMPLIES, IFF, FORALL, EXISTS, EQUALS_INFIX, NOT_EQUALS,
    WORD, VARIABLE
};

struct Token {
    TokenType type;
    std::string value;
};