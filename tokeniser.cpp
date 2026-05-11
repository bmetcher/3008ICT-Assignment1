#include "tokeniser.h"
#include <cctype>
#include <iostream>

void Tokeniser::skipWhitespaceAndComments() {
    while (pos < input.size()) {
        if (isspace(input[pos])) { // whitespace
            pos++;
        } else if (input[pos] == '%') { // comment
            while (pos < input.size() && input[pos] != '\n')
                pos++;
        } else if (input[pos] == '/' && // block comments
                   pos+1 < input.size() && 
                   input[pos+1] == '*') {
            pos+= 2;
            while (pos+1 < input.size() && 
                  !(input[pos] == '*' && input[pos+1] == '/'))
                pos++;
            pos+= 2;
            continue;
        } else {
            break;
        }
    }
    
}

Token Tokeniser::nextToken() {
    skipWhitespaceAndComments();
    // debugging
    //std::cerr << "[tok pos=" << pos << " char=" << (pos < input.size() ? input[pos] : '?') << "]\n";

    if (pos >= input.size()) // incase they forgot an END token
        return {TokenType::END, ""};

    char c = input[pos];

    // single character tokens
    if (c == '(') { pos++; return { TokenType::LPARENTH, "(" }; }
    if (c == ')') { pos++; return { TokenType::RPARENTH, ")" }; }
    if (c == '[') { pos++; return { TokenType::LBRACKET, "[" }; }
    if (c == ']') { pos++; return { TokenType::RBRACKET, "]" }; }
    if (c == ',') { pos++; return { TokenType::COMMA, "," }; }
    if (c == '.') { pos++; return { TokenType::PERIOD, "." }; }
    if (c == ':') { pos++; return { TokenType::COLON, ":" }; }
    if (c == '~') { pos++; return { TokenType::TILDE, "~" }; }
    if (c == '&') { pos++; return { TokenType::AND, "&" }; }
    if (c == '|') { pos++; return { TokenType::OR, "|" }; }
    if (c == '?') { pos++; return { TokenType::EXISTS, "?" }; }

    // equals and implies
    if (c == '=') {
        if (pos+1 < input.size() && input[pos+1] == '>') {
            pos += 2;
            return { TokenType::IMPLIES, "=>" };
        }
        pos++;
        return { TokenType::EQUALS_INFIX, "=" };
    }
    // not equals and ForAll
    if (c == '!') { 
        if (pos+1 < input.size() && input[pos+1] == '=') {
            pos += 2;
            return { TokenType:: NOT_EQUALS, "!=" };
        }
        pos++; return { TokenType::FORALL, "!" }; 
    }

    if (c == '<' && (pos+2 < input.size()) && input[pos+1] == '=' && input[pos+2] == '>') {
        pos += 3; return { TokenType::IFF, "<=>" };
    }

    // words and keywords
    if (isalpha(c) || c == '_') {
        std::string word;
        while (pos < input.size() && (isalnum(input[pos]) || input[pos] == '_'))
            word += input[pos++];

        if (isupper(word[0]))   return { TokenType::VARIABLE, word };
        if (word == "fof")      return { TokenType::FOF, word };
        if (word == "cnf")      return { TokenType::CNF, word };
        
        return { TokenType::WORD, word };
    }

    // numeric names
    if (isdigit(c)) {
        std::string num;
        while (pos < input.size() && isdigit(input[pos]))
            num += input[pos++];
        return { TokenType::WORD, num };
    }
    
    // unknown value
    return { TokenType::WORD, std::string(1, c) };
}