#ifndef FCI_ENGINE_H
#define FCI_ENGINE_H
#include <string>
#include <vector>

struct FciPagEdge {
    std::string left;
    std::string right;
    char leftMark;
    char rightMark;
};
struct FciPagResult {
    std::vector<std::string> variables;
    std::vector<FciPagEdge> edges;
    int observations;
};
bool runFci(const std::string& csvPath, double alpha, int maxConditioningSize,
            FciPagResult& result, std::string& error);
bool hasPagAdjacency(const FciPagResult& result, const std::string& a, const std::string& b);
bool hasDirectedPagEdge(const FciPagResult& result, const std::string& from, const std::string& to);
#endif
