#pragma once
#include <string>
#include <vector>
#include <memory>

/*
Terms:  Variable, Constant, Function
*/
struct Term {
    virtual ~Term() = default;
};
// X
struct Variable : Term {
    std::string name;
    Variable(std::string n) : name(n) {}
};
// a
struct Constant : Term {
    std::string name;
    Constant(std::string n) : name(n) {}
};
// f(X, a)
struct Function : Term {
    std::string name;
    std::vector<std::shared_ptr<Term>> args;
};

/*
Formulae: Predicate, Not, Or, Implies, ForAll, Exists
*/
struct Formula {
    virtual ~Formula() = default;
};

struct True : Formula {};
struct False : Formula {};

// p(X)     e.g.: friends(steve, eddie)  
struct Predicate : Formula {
    std::string name;
    std::vector<std::shared_ptr<Term>> args;
};

struct Not : Formula {
    std::shared_ptr<Formula> body;
};

struct And : Formula {
    std::shared_ptr<Formula> left;
    std::shared_ptr<Formula> right;
};

struct Or : Formula {
    std::shared_ptr<Formula> left;
    std::shared_ptr<Formula> right;
};

struct Iff : Formula {
    std::shared_ptr<Formula> left;
    std::shared_ptr<Formula> right;
};

struct Implies : Formula {
    std::shared_ptr<Formula> left;
    std::shared_ptr<Formula> right;
};

struct ForAll : Formula {
    std::string variable;
    std::shared_ptr<Formula> body;
};

struct Exists : Formula {
    std::string variable;
    std::shared_ptr<Formula> body;
};