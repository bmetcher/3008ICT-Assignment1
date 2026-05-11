#include "substitution.h"

// same for terms (needed because formulas contain terms)
std::shared_ptr<Term> substituteTerm(
    std::shared_ptr<Term> term, 
    const std::string& varName, 
    std::shared_ptr<Term> t
) {
    // if its the variable we're replacing, return t
    if (auto v = std::dynamic_pointer_cast<Variable>(term)) {
        if (v->name == varName)
            return t;
        return term; // different variable, leave it
    }  
    // constants are never replaced
    if (auto c = std::dynamic_pointer_cast<Constant>(term)) {
        return term;
    }
    // functions
    if (auto f = std::dynamic_pointer_cast<Function>(term)) {
        auto result = std::make_shared<Function>();
        result->name = f->name;
        for (auto& arg : f->args)
            result->args.push_back(substituteTerm(arg, varName, t)); // recursive
        return result;
    }
    return term;
}


std::shared_ptr<Formula> substitute(
    std::shared_ptr<Formula> formula,
    const std::string& varName,
    std::shared_ptr<Term> t
) {
    // Predicate
    if (auto p = std::dynamic_pointer_cast<Predicate>(formula)) {
        // substitute in each arg term
        auto result = std::make_shared<Predicate>();
        result->name = p->name;
        for (auto& arg : p->args)
            result->args.push_back(substituteTerm(arg, varName, t));
        return result;
    }
    
    // Not
    if (auto n = std::dynamic_pointer_cast<Not>(formula)) {
        auto result = std::make_shared<Not>();
        result->body = substitute(n->body, varName, t); 
        return result;
    }

    // And
    if (auto a = std::dynamic_pointer_cast<And>(formula)) {
        // recurse into a->left and a->right
        auto result = std::make_shared<And>();
        result->left    = substitute(a->left, varName, t);
        result->right   = substitute(a->right, varName, t);
        return result;
    }

    // Or
    if (auto o = std::dynamic_pointer_cast<Or>(formula)) {
        // recurse into a->left and a->right
        auto result = std::make_shared<Or>();
        result->left    = substitute(o->left, varName, t);
        result->right   = substitute(o->right, varName, t);
        return result;
    }

    // Implies
    if (auto i = std::dynamic_pointer_cast<Implies>(formula)) {
        // recurse into a->left and a->right
        auto result = std::make_shared<Implies>();
        result->left    = substitute(i->left, varName, t);
        result->right   = substitute(i->right, varName, t);
        return result;
    }

    // Iff
    if (auto iff = std::dynamic_pointer_cast<Iff>(formula)) {
        // recurse into a->left and a->right
        auto result = std::make_shared<Iff>();
        result->left    = substitute(iff->left, varName, t);
        result->right   = substitute(iff->right, varName, t);
        return result;
    }

    // ForAll
    if (auto fa = std::dynamic_pointer_cast<ForAll>(formula)) {
        if (fa->variable == varName) {
            return formula; // X is bound here; don't substitute
        }

        auto result = std::make_shared<ForAll>();
        result->variable = fa->variable;
        result->body = substitute(fa->body, varName, t);
        return result;
    }

    // Exists
    if (auto e = std::dynamic_pointer_cast<Exists>(formula)) {
        if (e->variable == varName) {
            return formula;
        }

        auto result = std::make_shared<Exists>();
        result->variable = e->variable;
        result->body = substitute(e->body, varName, t);
        return result;
    }

    return formula;
} 