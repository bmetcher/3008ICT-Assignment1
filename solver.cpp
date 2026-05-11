#include "solver.h"
#include "substitution.h"
#include <set>
#include <map>
#include <iostream>

// wall-clock timeout
std::chrono::time_point<std::chrono::high_resolution_clock> solveStartTime;
double solveTimeoutMs = 5000.0;

// fresh term generation
static int freshCounter = 0;
std::set<std::string> usedTerms;


std::shared_ptr<Term> freshTerm() {
    return std::make_shared<Constant>("_fresh" + std::to_string(freshCounter++));
}


void collectFromFormula(std::shared_ptr<Formula> f,
                        std::vector<std::shared_ptr<Term>>& terms)
{
    if (auto p = std::dynamic_pointer_cast<Predicate>(f)) {
        for (auto& arg : p->args)
            terms.push_back(arg);
        return;
    }
    if (auto n = std::dynamic_pointer_cast<Not>(f)) {
        collectFromFormula(n->body, terms);
        return;
    }
    if (auto a = std::dynamic_pointer_cast<And>(f)) {
        collectFromFormula(a->left, terms);
        collectFromFormula(a->right, terms);
        return;
    }
    if (auto o = std::dynamic_pointer_cast<Or>(f)) {
        collectFromFormula(o->left, terms);
        collectFromFormula(o->right, terms);
        return;
    }
    if (auto i = std::dynamic_pointer_cast<Implies>(f)) {
        collectFromFormula(i->left, terms);
        collectFromFormula(i->right, terms);
        return;
    }
    if (auto iff = std::dynamic_pointer_cast<Iff>(f)) {
        collectFromFormula(iff->left, terms);
        collectFromFormula(iff->right, terms);
        return;
    }
    if (auto fa = std::dynamic_pointer_cast<ForAll>(f)) {
        collectFromFormula(fa->body, terms);
        return;
    }
    if (auto e = std::dynamic_pointer_cast<Exists>(f)) {
        collectFromFormula(e->body, terms);
        return;
    }
}


std::vector<std::shared_ptr<Term>> collectTerms(const Sequent& s) {
    std::vector<std::shared_ptr<Term>> terms;
    for (auto& l : s.left) collectFromFormula(l, terms);
    for (auto& r : s.right) collectFromFormula(r, terms);
    return terms;
}


bool isClosed(const Sequent& s) {
    for (auto& f : s.right) {
        if (std::dynamic_pointer_cast<True>(f))
            return true;
    }
    for (auto& f : s.left) {
        if (std::dynamic_pointer_cast<False>(f))
            return true;
    }
    for (auto& l : s.left) {
        for (auto& r : s.right) {
            if (formulaEquals(l, r))
                return true;
        }
    }
    return false;
}


bool formulaEquals(std::shared_ptr<Formula> a, std::shared_ptr<Formula> b) {
    if (auto pa = std::dynamic_pointer_cast<Predicate>(a)) {
        if (auto pb = std::dynamic_pointer_cast<Predicate>(b)) {
            if (pa->name != pb->name) return false;
            if (pa->args.size() != pb->args.size()) return false;
            for (size_t i = 0; i < pa->args.size(); i++) {
                if (!termEquals(pa->args[i], pb->args[i]))
                    return false;
            }
            return true;
        }
        return false;
    }
    if (auto na = std::dynamic_pointer_cast<Not>(a)) {
        if (auto nb = std::dynamic_pointer_cast<Not>(b)) {
            return formulaEquals(na->body, nb->body);
        }
        return false;
    }
    if (auto aa = std::dynamic_pointer_cast<And>(a)) {
        if (auto ab = std::dynamic_pointer_cast<And>(b)) {
            return formulaEquals(aa->left, ab->left)
                && formulaEquals(aa->right, ab->right);
        }
        return false;
    }
    if (auto oa = std::dynamic_pointer_cast<Or>(a)) {
        if (auto ob = std::dynamic_pointer_cast<Or>(b)) {
            return formulaEquals(oa->left, ob->left)
                && formulaEquals(oa->right, ob->right);
        }
        return false;
    }
    if (auto ia = std::dynamic_pointer_cast<Implies>(a)) {
        if (auto ib = std::dynamic_pointer_cast<Implies>(b)) {
            return formulaEquals(ia->left, ib->left)
                && formulaEquals(ia->right, ib->right);
        }
        return false;
    }
    if (auto ia = std::dynamic_pointer_cast<Iff>(a)) {
        if (auto ib = std::dynamic_pointer_cast<Iff>(b)) {
            return formulaEquals(ia->left, ib->left)
                && formulaEquals(ia->right, ib->right);
        }
        return false;
    }
    if (auto fa = std::dynamic_pointer_cast<ForAll>(a)) {
        if (auto fb = std::dynamic_pointer_cast<ForAll>(b)) {
            return fa->variable == fb->variable
                && formulaEquals(fa->body, fb->body);
        }
        return false;
    }
    if (auto ea = std::dynamic_pointer_cast<Exists>(a)) {
        if (auto eb = std::dynamic_pointer_cast<Exists>(b)) {
            return ea->variable == eb->variable
                && formulaEquals(ea->body, eb->body);
        }
        return false;
    }
    return false;
}


bool termEquals(std::shared_ptr<Term> a, std::shared_ptr<Term> b) {
    if (auto va = std::dynamic_pointer_cast<Variable>(a)) {
        if (auto vb = std::dynamic_pointer_cast<Variable>(b)) {
            return va->name == vb->name;
        }
        return false;
    }
    if (auto ca = std::dynamic_pointer_cast<Constant>(a)) {
        if (auto cb = std::dynamic_pointer_cast<Constant>(b)) {
            return ca->name == cb->name;
        }
        return false;
    }
    if (auto fa = std::dynamic_pointer_cast<Function>(a)) {
        if (auto fb = std::dynamic_pointer_cast<Function>(b)) {
            if (fa->name != fb->name) return false;
            if (fa->args.size() != fb->args.size()) return false;
            for (size_t i = 0; i < fa->args.size(); i++) {
                if (!termEquals(fa->args[i], fb->args[i])) return false;
            }
            return true;
        }
        return false;
    }
    return false;
}


// Improved solver: depth-first proof search with bounded gamma applications.
// gammaCount tracks how many gamma rule applications have happened on this branch.
// gammaBudget is the cap for the current iteration of iterative deepening.
// When gammaCount >= gammaBudget, gamma rules are blocked and the branch returns Failed.
Result solveSequent(Sequent sequent, int depth, int gammaCount, int gammaBudget) {
    // wall-clock timeout check (sampled to avoid overhead)
    static int checkCounter = 0;
    if (++checkCounter % 1000 == 0) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<double, std::milli>(now - solveStartTime).count();
        if (elapsed > solveTimeoutMs) return Result::Timeout;
    }

    // total depth guard against runaway non-gamma recursion
    if (depth > 500) return Result::Failed;

    // 1. Closure
    if (isClosed(sequent))
        return Result::Proved;

    // 2. Single-premise invertible rules (don't count toward gamma budget)
    // And L
    for (size_t i = 0; i < sequent.left.size(); i++) {
        if (auto a = std::dynamic_pointer_cast<And>(sequent.left[i])) {
            Sequent next = sequent;
            next.left.erase(next.left.begin() + i);
            next.left.push_back(a->left);
            next.left.push_back(a->right);
            return solveSequent(next, depth + 1, gammaCount, gammaBudget);
        }
    }
    // Not L
    for (size_t i = 0; i < sequent.left.size(); i++) {
        if (auto n = std::dynamic_pointer_cast<Not>(sequent.left[i])) {
            Sequent next = sequent;
            next.left.erase(next.left.begin() + i);
            next.right.push_back(n->body);
            return solveSequent(next, depth + 1, gammaCount, gammaBudget);
        }
    }
    // Not R
    for (size_t i = 0; i < sequent.right.size(); i++) {
        if (auto n = std::dynamic_pointer_cast<Not>(sequent.right[i])) {
            Sequent next = sequent;
            next.right.erase(next.right.begin() + i);
            next.left.push_back(n->body);
            return solveSequent(next, depth + 1, gammaCount, gammaBudget);
        }
    }
    // Or R
    for (size_t i = 0; i < sequent.right.size(); i++) {
        if (auto o = std::dynamic_pointer_cast<Or>(sequent.right[i])) {
            Sequent next = sequent;
            next.right.erase(next.right.begin() + i);
            next.right.push_back(o->left);
            next.right.push_back(o->right);
            return solveSequent(next, depth + 1, gammaCount, gammaBudget);
        }
    }
    // Implies R
    for (size_t i = 0; i < sequent.right.size(); i++) {
        if (auto imp = std::dynamic_pointer_cast<Implies>(sequent.right[i])) {
            Sequent next = sequent;
            next.right.erase(next.right.begin() + i);
            next.left.push_back(imp->left);
            next.right.push_back(imp->right);
            return solveSequent(next, depth + 1, gammaCount, gammaBudget);
        }
    }
    // ForAll R (delta — invertible due to fresh term)
    for (size_t i = 0; i < sequent.right.size(); i++) {
        if (auto f = std::dynamic_pointer_cast<ForAll>(sequent.right[i])) {
            Sequent next = sequent;
            next.right.erase(next.right.begin() + i);
            auto fresh = freshTerm();
            next.right.push_back(substitute(f->body, f->variable, fresh));
            return solveSequent(next, depth + 1, gammaCount, gammaBudget);
        }
    }
    // Exists L (delta — same)
    for (size_t i = 0; i < sequent.left.size(); i++) {
        if (auto e = std::dynamic_pointer_cast<Exists>(sequent.left[i])) {
            Sequent next = sequent;
            next.left.erase(next.left.begin() + i);
            auto fresh = freshTerm();
            next.left.push_back(substitute(e->body, e->variable, fresh));
            return solveSequent(next, depth + 1, gammaCount, gammaBudget);
        }
    }

    // 3. Branching invertible rules (don't count toward gamma budget)
    // And R
    for (size_t i = 0; i < sequent.right.size(); i++) {
        if (auto a = std::dynamic_pointer_cast<And>(sequent.right[i])) {
            Sequent lb = sequent, rb = sequent;
            lb.right.erase(lb.right.begin() + i);
            lb.right.push_back(a->left);
            rb.right.erase(rb.right.begin() + i);
            rb.right.push_back(a->right);

            Result lr = solveSequent(lb, depth + 1, gammaCount, gammaBudget);
            if (lr == Result::Timeout) return Result::Timeout;
            if (lr == Result::Failed)  return Result::Failed;
            Result rr = solveSequent(rb, depth + 1, gammaCount, gammaBudget);
            return rr;
        }
    }
    // Or L
    for (size_t i = 0; i < sequent.left.size(); i++) {
        if (auto o = std::dynamic_pointer_cast<Or>(sequent.left[i])) {
            Sequent lb = sequent, rb = sequent;
            lb.left.erase(lb.left.begin() + i);
            lb.left.push_back(o->left);
            rb.left.erase(rb.left.begin() + i);
            rb.left.push_back(o->right);

            Result lr = solveSequent(lb, depth + 1, gammaCount, gammaBudget);
            if (lr == Result::Timeout) return Result::Timeout;
            if (lr == Result::Failed)  return Result::Failed;
            Result rr = solveSequent(rb, depth + 1, gammaCount, gammaBudget);
            return rr;
        }
    }
    // Implies L
    for (size_t i = 0; i < sequent.left.size(); i++) {
        if (auto imp = std::dynamic_pointer_cast<Implies>(sequent.left[i])) {
            Sequent lb = sequent, rb = sequent;
            lb.left.erase(lb.left.begin() + i);
            lb.right.push_back(imp->left);
            rb.left.erase(rb.left.begin() + i);
            rb.left.push_back(imp->right);

            Result lr = solveSequent(lb, depth + 1, gammaCount, gammaBudget);
            if (lr == Result::Timeout) return Result::Timeout;
            if (lr == Result::Failed)  return Result::Failed;
            Result rr = solveSequent(rb, depth + 1, gammaCount, gammaBudget);
            return rr;
        }
    }

    // *** GAMMA BUDGET CHECK ***
    // Beyond this point, all rules count as gamma applications.
    // If we've used our budget, fail this branch — iterative deepening retries with a higher budget.
    if (gammaCount >= gammaBudget) {
        return Result::Failed;
    }

    // 4. Gamma rules with existing terms
    // ForAll L
    for (size_t i = 0; i < sequent.left.size(); i++) {
        if (auto f = std::dynamic_pointer_cast<ForAll>(sequent.left[i])) {
            auto terms = collectTerms(sequent);
            for (auto& t : terms) {
                std::string tname;
                if (auto c = std::dynamic_pointer_cast<Constant>(t)) tname = c->name;
                else if (auto v = std::dynamic_pointer_cast<Variable>(t)) tname = v->name;
                else continue;
                if (sequent.usedInstantiations[sequent.left[i]].count(tname))
                    continue;
                Sequent next = sequent;
                next.usedInstantiations[sequent.left[i]].insert(tname);
                next.left.push_back(substitute(f->body, f->variable, t));
                Result r = solveSequent(next, depth + 1, gammaCount + 1, gammaBudget);
                if (r == Result::Timeout) return Result::Timeout;
                if (r == Result::Proved)  return Result::Proved;
            }
        }
    }
    // Exists R
    for (size_t i = 0; i < sequent.right.size(); i++) {
        if (auto e = std::dynamic_pointer_cast<Exists>(sequent.right[i])) {
            auto terms = collectTerms(sequent);
            for (auto& t : terms) {
                std::string tname;
                if (auto c = std::dynamic_pointer_cast<Constant>(t)) tname = c->name;
                else if (auto v = std::dynamic_pointer_cast<Variable>(t)) tname = v->name;
                else continue;
                if (sequent.usedInstantiations[sequent.right[i]].count(tname))
                    continue;
                Sequent next = sequent;
                next.usedInstantiations[sequent.right[i]].insert(tname);
                next.right.push_back(substitute(e->body, e->variable, t));
                Result r = solveSequent(next, depth + 1, gammaCount + 1, gammaBudget);
                if (r == Result::Timeout) return Result::Timeout;
                if (r == Result::Proved)  return Result::Proved;
            }
        }
    }

    // 5. Gamma rules with fresh term
    // ForAll L
    for (size_t i = 0; i < sequent.left.size(); i++) {
        if (auto f = std::dynamic_pointer_cast<ForAll>(sequent.left[i])) {
            auto fresh = freshTerm();
            std::string fname = std::dynamic_pointer_cast<Constant>(fresh)->name;
            if (sequent.usedInstantiations[sequent.left[i]].count(fname))
                continue;
            Sequent next = sequent;
            next.usedInstantiations[sequent.left[i]].insert(fname);
            next.left.push_back(substitute(f->body, f->variable, fresh));
            Result r = solveSequent(next, depth + 1, gammaCount + 1, gammaBudget);
            if (r == Result::Timeout) return Result::Timeout;
            if (r == Result::Proved)  return Result::Proved;
        }
    }
    // Exists R
    for (size_t i = 0; i < sequent.right.size(); i++) {
        if (auto e = std::dynamic_pointer_cast<Exists>(sequent.right[i])) {
            auto fresh = freshTerm();
            std::string fname = std::dynamic_pointer_cast<Constant>(fresh)->name;
            if (sequent.usedInstantiations[sequent.right[i]].count(fname))
                continue;
            Sequent next = sequent;
            next.usedInstantiations[sequent.right[i]].insert(fname);
            next.right.push_back(substitute(e->body, e->variable, fresh));
            Result r = solveSequent(next, depth + 1, gammaCount + 1, gammaBudget);
            if (r == Result::Timeout) return Result::Timeout;
            if (r == Result::Proved)  return Result::Proved;
        }
    }

    // 6. No rule applies
    return Result::Failed;
}


// Iterative deepening over gamma budget.
// Tries to prove with budget 1, then 2, then 3, etc.
// This addresses the gamma-rule fairness problem of naive Algorithm 2.
Result iterativeDeepening(Sequent sequent, int maxGammaBudget) {
    for (int budget = 1; budget <= maxGammaBudget; budget++) {
        Result r = solveSequent(sequent, 0, 0, budget);
        if (r == Result::Proved)  return Result::Proved;
        if (r == Result::Timeout) return Result::Timeout;
        // Failed → try next budget

        // wall-clock check between iterations
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<double, std::milli>(now - solveStartTime).count();
        if (elapsed > solveTimeoutMs) return Result::Timeout;
    }
    return Result::Failed;
}