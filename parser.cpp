#include "parser.h"
#include "tokeniser.h"
#include <stdexcept>
#include <iostream>     // error finding

bool Parser::hasMore() {
    return peek().type != TokenType::END;
}

// look at current token without consuming it
Token Parser::peek() {
    return current;
}

// consume current token and advance
Token Parser::consume() {
    Token t = current;
    current = tokeniser.nextToken();
    return t;
}

// consume but throw error if it's unexpected
Token Parser::expect(TokenType t) {
    if (current.type != t)
        throw std::runtime_error("unexpected token: " + current.value);
    return consume();
}

/* 
fof(name, role, formula).

Go by binding order:
1. <=>
2. =>
3. | and &
4. ~
5. Atoms and Quantifiers
*/
ProofProblem Parser::parseFOF() {
    expect(TokenType::FOF);
    expect(TokenType::LPARENTH);
    std::string name = expect(TokenType::WORD).value;    // name, like "test1"
    std::cerr << "parsing formula: " << name << "\n";   // error checking
    expect(TokenType::COMMA);
    std::string role = expect(TokenType::WORD).value;    // role, like "conjecture"
    expect(TokenType::COMMA);
    auto formula = parseFormula();  // actual content
    expect(TokenType::RPARENTH);
    expect(TokenType::PERIOD);
    return {name, role, formula};
}

// handle <=>
std::shared_ptr<Formula> Parser::parseFormula() {
    //std::cerr << "[parseFormula]\n"; // debugging
    auto left = parseBinary();

    if (peek().type == TokenType::IFF) {
        consume(); // consume <=>
        auto right = parseFormula(); // recurse RHS
        auto fwd = std::make_shared<Implies>();
        fwd->left = left;
        fwd->right = right;
        auto bwd = std::make_shared<Implies>();
        bwd->left = right;
        bwd->right = left;
        auto node = std::make_shared<And>();
        node->left = fwd;
        node->right = bwd;
        return node;
    }

    return left; // no <=> means pass thru
}

// handle =>
std::shared_ptr<Formula> Parser::parseBinary() {
    auto left = parseUnary();
    //std::cerr << "[parseBinary]\n"; // debugging

    if (peek().type == TokenType::IMPLIES) {
        consume(); // consume =>
        auto right = parseBinary(); // recurse RHS
        auto node = std::make_shared<Implies>();
        node->left = left;
        node->right = right;
        return node;
    }

    return left;
}

// handle & and |
std::shared_ptr<Formula> Parser::parseUnary() {
    auto left = parseNot();
    //std::cerr << "[parseUnary]\n"; // debugging

    if (peek().type == TokenType::AND) {
        consume();  // consume &
        auto right = parseUnary();
        auto node = std::make_shared<And>();
        node->left = left;
        node->right = right;
        return node;
    }

    if (peek().type == TokenType::OR) {
        consume();  // consume |
        auto right = parseUnary();
        auto node = std::make_shared<Or>();
        node->left = left;
        node->right = right;
        return node;
    }

    return left;
}

// handles ~
std::shared_ptr<Formula> Parser::parseNot() {
    //std::cerr << "[parseNot]\n"; // debugging
    if (peek().type == TokenType::TILDE) {
        consume(); // consume the ~
        auto node = std::make_shared<Not>();
        node->body = parseNot(); // recurse to handle "~~p"
        return node;
    }

    return parseUnitary();
}

// handle () and quantifiers
std::shared_ptr<Formula> Parser::parseUnitary() {
    //std::cerr << "[parseUnitary]\n"; // debugging

    // case 0: true or false
    if (peek().value == "$true") {
        consume();
        return std::make_shared<True>();
    }
    if (peek().value == "$false") {
        consume();
        return std::make_shared<False>();
    }

    // case 1: parenthesised formula  (A => B)
    if (peek().type == TokenType::LPARENTH) {
        consume();
        auto f = parseFormula();
        expect(TokenType::RPARENTH);
        return f;
    }

    // case 2: quantified  ![X]: body  or  ?[X]: body
    if (peek().type == TokenType::FORALL || peek().type == TokenType::EXISTS) {
        return parseQuantified();
    }

    // try parsing a term (might be start of X = Y or X != Y)
    if (peek().type == TokenType::VARIABLE || peek().type == TokenType::WORD) {
        auto term = parseTerm();

        if (peek().type == TokenType::EQUALS_INFIX) {
            consume();
            auto rhs = parseTerm();
            // represent as Predicate("=", [lhs, rhs])
            auto node = std::make_shared<Predicate>();
            node->name = "=";
            node->args.push_back(term);
            node->args.push_back(rhs);
            return node;
        }

        if (peek().type == TokenType::NOT_EQUALS) {
            consume();
            auto rhs = parseTerm();
            // represent as Not(Predicate("=", [lhs, rhs]))
            auto eq = std::make_shared<Predicate>();
            eq->name = "=";
            eq->args.push_back(term);
            eq->args.push_back(rhs);
            auto node = std::make_shared<Not>();
            node->body = eq;
            return node;
        }

        // not equality; must be predicate, but we already consumed name
        if (auto c = std::dynamic_pointer_cast<Constant>(term)) {
            auto node = std::make_shared<Predicate>();
            node->name = c->name;
            if (peek().type == TokenType::LPARENTH) {
                consume();
                node->args.push_back(parseTerm());
                while (peek().type == TokenType::COMMA) {
                    consume();
                    node->args.push_back(parseTerm());
                }
                expect(TokenType::RPARENTH);
            }
            return node;
        }

        // if parseTerm returned a Function, it already consumed the args
        // just wrap it as a predicate
        if (auto f = std::dynamic_pointer_cast<Function>(term)) {
            auto node = std::make_shared<Predicate>();
            node->name = f->name;
            node->args = f->args;   // args already passed
            return node;
        }

        // error checking
        throw std::runtime_error("expected predicate or equality; got variable: "
            + peek().value + " after term: " +
            (std::dynamic_pointer_cast<Variable>(term) ?
             std::dynamic_pointer_cast<Variable>(term)->name : "?"));
    }

    // case 3: atom  p(X, a)
    return parsePredicate();
}

std::shared_ptr<Formula> Parser::parseQuantified() {
    //std::cerr << "[parseQuantified]\n"; // debugging

    bool isForAll = (peek().type == TokenType::FORALL);
    consume();                      // consume ! or ?
    expect(TokenType::LBRACKET);    // [

    // read comma-separated variables
    std::vector<std::string> vars;
    vars.push_back(expect(TokenType::VARIABLE).value);
    while (peek().type == TokenType::COMMA) {
        consume();
        vars.push_back(expect(TokenType::VARIABLE).value);
    }

    expect(TokenType::RBRACKET);    // ]
    expect(TokenType::COLON);       // :
    auto body = parseFormula();

    // wrap from right to left -- ![X,Y]: body becomes ForAll(X, ForAll(Y, body))
    std::shared_ptr<Formula> result = body;
    for (int i = vars.size() - 1; i >= 0; i--) {
        if (isForAll) {
            auto node = std::make_shared<ForAll>();
            node->variable = vars[i];
            node->body = result;
            result = node;
        } else {
            auto node = std::make_shared<Exists>();
            node->variable = vars[i];
            node->body = result;
            result = node;
        }
    }
    return result;
}

// p(X, a)  or just  p
std::shared_ptr<Formula> Parser::parsePredicate() {
    //std::cerr << "[parsePredicate]\n"; // debugging

    std::string name = expect(TokenType::WORD).value;   // consume name

    auto node = std::make_shared<Predicate>();
    node->name = name;

    // check if there are arguments
    if (peek().type == TokenType::LPARENTH) {
        consume();                          // consume (
        node->args.push_back(parseTerm());  // first arg
        while (peek().type == TokenType::COMMA) {
            consume();                          // consume ,
            node->args.push_back(parseTerm());   // next arg
        }
        expect(TokenType::RPARENTH);        // consume )
    }

    return node;
}

std::shared_ptr<Term> Parser::parseTerm() {
    //std::cerr << "[parseTerm]\n"; // debugging

    // variable
    if (peek().type == TokenType::VARIABLE) {
        std::string name = consume().value;
        return std::make_shared<Variable>(name);
    }

    // constant or function
    if (peek().type == TokenType::WORD) {
        std::string name = consume().value;

        // if followed by '(' its a function
        if (peek().type == TokenType::LPARENTH) {
            consume();  // consume (
            auto node = std::make_shared<Function>();
            node->name = name;
            node->args.push_back(parseTerm());  // first arg
            while (peek().type == TokenType::COMMA) {
                consume();                      // eat ,
                node->args.push_back(parseTerm());
            }
            expect(TokenType::RPARENTH);        // eat )
            return node;
        }

        return std::make_shared<Constant>(name);
    }

    throw std::runtime_error("expected term, got: " + peek().value);
}
