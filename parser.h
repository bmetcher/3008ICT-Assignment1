#pragma once
#include <memory>
#include <string>
#include "tokeniser.h"
#include "ast.h"

struct ProofProblem {
    std::string name;
    std::string role;
    std::shared_ptr<Formula> formula;
};

class Parser {
public:
    Parser(Tokeniser& tokeniser) : tokeniser(tokeniser) {
        current = tokeniser.nextToken();    // initialise with first token
    }

    ProofProblem parseFOF();
    bool hasMore();

private:
    Tokeniser& tokeniser;
    Token current;

    Token peek();
    Token consume();
    Token expect(TokenType t);

    std::shared_ptr<Formula> parseFormula();
    std::shared_ptr<Formula> parseBinary();
    std::shared_ptr<Formula> parseUnary();
    std::shared_ptr<Formula> parseUnitary();
    std::shared_ptr<Formula> parseNot();
    std::shared_ptr<Formula> parseQuantified();
    std::shared_ptr<Formula> parsePredicate();
    std::shared_ptr<Term>    parseTerm();
};