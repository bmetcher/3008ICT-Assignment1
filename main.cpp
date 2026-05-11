#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <cmath>
#include "tokeniser.h"
#include "parser.h"
#include "substitution.h"
#include "solver.h"


int main() {
    std::ifstream file("test.p"); // e.g.: fof(test, conjecture, ![X]: (p(X) => q(a))).
    std::string input((std::istreambuf_iterator<char>(file)), 
                       std::istreambuf_iterator<char>());

    Tokeniser tokeniser(input);
    Parser parser(tokeniser);

    const char* status; // for tracking proved/failed/timeout
    const double timeoutMs = 5000.0; // 5 second limit per problem

    // collect all problems first
    std::vector<ProofProblem> problems;
    try {
        while (parser.hasMore())
            problems.push_back(parser.parseFOF());
    } catch (std::runtime_error& e) {
        std::cout << "parse error: " << e.what() << "\n";
        return 1;
    }
    
    // load initial axioms and conjectures
    std::vector<std::shared_ptr<Formula>> axioms;
    std::vector<ProofProblem> conjectures;
    for (auto& p : problems) {
        if (p.role == "axiom" || p.role == "hypothesis")
            axioms.push_back(p.formula);
        else if (p.role == "conjecture") {
            conjectures.push_back(p);
        }
    }

    // prove each conjecture against the shared axioms
    int proved = 0, failed = 0, timeouts = 0;
    std::vector<double> times;
    std::vector<std::string> timeoutNames;
    std::vector<std::string> failedNames;


    for (auto& conjecture : conjectures) {
        Sequent initial;
        initial.left = axioms;  // same axioms each for each conjecture
        initial.right.push_back(conjecture.formula);

        // record time and start solving
        solveStartTime  = std::chrono::high_resolution_clock::now();
        auto start      = solveStartTime;
        Result result   = iterativeDeepening(initial, 20);  // 20 for max gamma budget
        auto end        = std::chrono::high_resolution_clock::now();
        double ms       = std::chrono::duration<double, std::milli>(end - start).count();
        
        // handle result
        switch (result) {
            case Result::Proved:    status = "proved"; proved++; break;
            case Result::Failed:    status = "failed"; failed++; 
                                    failedNames.push_back(conjecture.name); break;
            case Result::Timeout:   status = "timeout"; timeouts++; 
                                    timeoutNames.push_back(conjecture.name); break;
        }

        // only add scores for proved or failed sequents
        if (result != Result::Timeout) times.push_back(ms);

        std::cout   << conjecture.name << ": " << status
                    << " in " << ms << "ms\n";
    }

    // Statistics
    int n = times.size();
    if (n > 0) {
        // average
        double sum = 0;
        for (double t : times) sum += t;
        double mean = sum / n;

        // standard deviation
        double sq_sum = 0;
        for (double t : times) sq_sum += (t - mean) * (t - mean);
        double std_dev = std::sqrt(sq_sum / n);

        // 95% confidence interval (1.96 * std_err)
        double std_err  = std_dev / std::sqrt(n);
        double ci95     = 1.96 * std_err;

        // Print statistics (proved, failed (+list), timeouts(+list), total, and time results)
        std::cout << "\n--- Results ---\n";
        std::cout << "Proved:   " << proved << "\n";
        std::cout << "Failed:   " << failed << "\n";
        if (!failedNames.empty()) {
            std::cout << " (";
            for (size_t i = 0; i < failedNames.size(); i++) {
                std::cout << failedNames[i];
                if (i + 1 < failedNames.size()) std::cout << ", ";
            }
            std::cout << ")\n";
        }
        std::cout << "Timeouts: " << timeouts << "\n";
        if (!timeoutNames.empty()) {
            std::cout << " (";
            for (size_t i = 0; i < timeoutNames.size(); i++) {
                std::cout << timeoutNames[i];
                if (i + 1 < timeoutNames.size()) std::cout << ", ";
            }
            std::cout << ")\n";
        }
        std::cout << "Total:    " << conjectures.size() << "\n";
        std::cout << "Avg time: " << mean << "ms\n";
        std::cout << "Std. Dev: " << std_dev << "ms\n";
        std::cout << "95% CI:   " << mean << "+/- " << ci95 << "ms\n";
    }

    return 0;
}