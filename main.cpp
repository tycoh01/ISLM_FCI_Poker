#include "fci_engine.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

struct Opponent {
    std::string name;
    int exportYear;
    double exportsUsdBillion;
    double usDestinationSharePct;
    std::string conservativePlay;
};

bool named(const FciPagResult& result, const std::string& variable) {
    for (size_t i = 0; i < result.variables.size(); ++i)
        if (result.variables[i] == variable) return true;
    return false;
}

void showPolicyInstrument(const FciPagResult& pag, const std::string& instrument) {
    const std::string outcome = "GDPGrowth";
    if (!named(pag, instrument) || !named(pag, outcome)) {
        std::cout << "  " << instrument << ": absent from input data; fold quantitative claim.\n";
    } else if (hasDirectedPagEdge(pag, instrument, outcome)) {
        std::cout << "  " << instrument << ": PAG has a direction toward GDP growth; allow only a small, reversible pilot, not a forecast.\n";
    } else if (hasPagAdjacency(pag, instrument, outcome)) {
        std::cout << "  " << instrument << ": PAG adjacency is unresolved; call/monitor, but do not claim a GDP effect.\n";
    } else {
        std::cout << "  " << instrument << ": no retained PAG adjacency with GDP growth; fold the quantitative intervention claim.\n";
    }
}

int main(int argc, char** argv) {
    const std::string dataPath = argc > 1 ? argv[1] : "fci_real_input.csv";
    const double alpha = argc > 2 ? std::atof(argv[2]) : 0.05;
    const int maxConditioning = argc > 3 ? std::atoi(argv[3]) : 2;

    FciPagResult pag;
    std::string error;
    if (!runFci(dataPath, alpha, maxConditioning, pag, error)) {
        std::cerr << "FCI failed: " << error << "\n"
                  << "Usage: PokerAI-ISLM.exe [observed_data.csv] [alpha=0.05] [max_conditioning=2]\n";
        return 2;
    }

    std::cout << "Poker AI + FCI conservative policy rule for the IS-LM context\n"
              << "Observed transformed U.S. macro data: " << pag.observations << " quarters; alpha="
              << alpha << "; max conditioning size=" << maxConditioning << "\n\n"
              << "FCI PAG retained edges (circle endpoints remain unresolved):\n";
    for (size_t i = 0; i < pag.edges.size(); ++i) {
        const FciPagEdge& edge = pag.edges[i];
        std::cout << "  " << edge.left << " " << edge.leftMark << "-" << edge.rightMark << " " << edge.right << "\n";
    }
    std::cout << "\nObserved-history IS reduced-form reference (not causal):\n"
              << "  GDP growth = intercept + 0.0541 x lagged real rate + 0.0464 x government-spending growth\n"
              << "  n=200, R-squared=0.013; the historical fit is weak.\n"
              << "  Conservative FCI gate (policy instrument -> GDP growth):\n";
    showPolicyInstrument(pag, "RealInterestRate");
    showPolicyInstrument(pag, "GovtGrowth");
    std::cout << "  Rule: only a PAG-directed policy-to-GDP edge permits a small pilot. An unresolved or absent edge is not evidence of zero effect; it means this data/model does not justify a numerical causal claim.\n";

    const Opponent opponents[] = {
        {"China", 2024, 3576.5433, 14.7, "Hedge trade exposure, diversify critical inputs, and keep channels open."},
        {"European Union", 2025, 2960.9060, 21.0, "Prefer negotiation and reversible support; avoid broad retaliatory measures."},
        {"Japan", 2025, 708.4944, 18.6, "Protect supply-chain continuity and use targeted, reversible support."}
    };
    std::cout << "\nPoker AI conservative policy plays by counterpart\n"
              << "(Observed U.S. destination share of counterpart exports; not a U.S. import-dependence measure.)\n";
    for (int i = 0; i < 3; ++i) {
        std::cout << "\n  " << opponents[i].name << " | reported export year " << opponents[i].exportYear
                  << " | exports USD " << std::fixed << std::setprecision(1) << opponents[i].exportsUsdBillion
                  << "B | U.S. destination share " << opponents[i].usDestinationSharePct << "%\n"
                  << "  Conservative play: " << opponents[i].conservativePlay << "\n";
    }
    std::cout << "\nConservative IS-LM conclusion:\n"
              << "  Keep automatic stabilizers in place; do not assign a GDP multiplier to a new discretionary rate or spending move from this PAG.\n"
              << "  Monitor observed GDP, consumption, and investment links; use new, current data before considering a reversible policy pilot.\n"
              << "  Export years differ across counterparts. Trade profiles and old U.S. macro estimates are descriptive, not 2027 forecasts.\n";
    return 0;
}

