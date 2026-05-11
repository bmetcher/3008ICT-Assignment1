#pragma once
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include "ast.h"
#include <chrono>

// possible result classifiers
enum class Result { Proved, Failed, Timeout };
// tracking time-based timeouts (global.. ugly but functioning)
extern std::chrono::time_point<std::chrono::high_resolution_clock> solveStartTime;
extern double solvetimeoutMs;

// data structures
struct Sequent {
    std::vector<std::shared_ptr<Formula>> left;     // Gamma / left
    std::vector<std::shared_ptr<Formula>> right;    // Delta / right
    std::map<std::shared_ptr<Formula>, std::set<std::string>> usedInstantiations;
};

struct ProofNode {
    Sequent sequent;
    std::vector<std::shared_ptr<ProofNode>> premises; // children (upward)
    bool closed = false;
};

std::shared_ptr<Term> freshTerm();
std::vector<std::shared_ptr<Term>> collectTerms(const Sequent& s);

bool prove(std::shared_ptr<Formula> formula);
bool isClosed(const Sequent& s);
bool formulaEquals(std::shared_ptr<Formula> a, std::shared_ptr<Formula> b);
bool termEquals(std::shared_ptr<Term> a, std::shared_ptr<Term> b);
void collectFromFormula(std::shared_ptr<Formula> f,
                        std::vector<std::shared_ptr<Term>>& terms);
Result solveSequent(Sequent sequent, int depth = 0);

// Improvements (budget is so high; it acts like unbounded baseline with a specific value)
Result solveSequent(Sequent sequent, int depth = 0, int gammaCount = 0, int gammaBudget = 999999);
Result iterativeDeepening(Sequent sequent, int maxGammaBudget = 20);

