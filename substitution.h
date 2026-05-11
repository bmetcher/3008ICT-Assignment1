#pragma once
#include <memory>
#include <string>
#include "ast.h"

// forward declarations
std::shared_ptr<Term> substituteTerm(
    std::shared_ptr<Term> term,
    const std::string& varName,
    std::shared_ptr<Term> t
);

std::shared_ptr<Formula> substitute(
    std::shared_ptr<Formula> formula,
    const std::string& varName,
    std::shared_ptr<Term> t
);