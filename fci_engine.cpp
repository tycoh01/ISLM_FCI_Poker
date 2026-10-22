#include "fci_engine.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace {
enum Mark { NONE = 0, CIRCLE = 1, TAIL = 2, ARROW = 3 };
typedef std::vector<std::vector<double> > Matrix;
typedef std::vector<int> Set;

struct Data { std::vector<std::string> names; Matrix values; };
struct Graph {
    std::vector<std::vector<int> > mark;
    explicit Graph(int n) : mark(n, std::vector<int>(n, NONE)) {
        for (int i = 0; i < n; ++i) for (int j = i + 1; j < n; ++j)
            mark[i][j] = mark[j][i] = CIRCLE;
    }
    bool adjacent(int a, int b) const { return mark[a][b] != NONE; }
    std::vector<int> neighbors(int a) const {
        std::vector<int> r; for (size_t i = 0; i < mark.size(); ++i) if (adjacent(a, (int)i)) r.push_back((int)i); return r;
    }
    void remove(int a, int b) { mark[a][b] = mark[b][a] = NONE; }
    void orientAt(int from, int at, int endpoint) { mark[from][at] = mark[at][from] == NONE ? NONE : mark[from][at]; mark[at][from] = endpoint; }
};

bool readCsv(const std::string& path, Data& d) {
    std::ifstream in(path.c_str()); if (!in) return false;
    std::string line; if (!std::getline(in, line)) return false;
    std::stringstream hs(line); std::string cell;
    while (std::getline(hs, cell, ',')) d.names.push_back(cell);
    if (d.names.size() < 2) return false;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line); std::vector<double> row;
        while (std::getline(ss, cell, ',')) { char* end = NULL; double v = std::strtod(cell.c_str(), &end); if (end == cell.c_str() || *end != '\0') return false; row.push_back(v); }
        if (row.size() != d.names.size()) return false;
        d.values.push_back(row);
    }
    return d.values.size() >= 5;
}

Matrix correlations(const Data& d) {
    const int p = (int)d.names.size(); const double n = (double)d.values.size();
    std::vector<double> mean(p, 0.0), sd(p, 0.0); Matrix r(p, std::vector<double>(p, 0.0));
    for (int j = 0; j < p; ++j) for (size_t i = 0; i < d.values.size(); ++i) mean[j] += d.values[i][j] / n;
    for (int j = 0; j < p; ++j) for (size_t i = 0; i < d.values.size(); ++i) sd[j] += (d.values[i][j]-mean[j])*(d.values[i][j]-mean[j]);
    for (int j = 0; j < p; ++j) sd[j] = std::sqrt(sd[j] / (n - 1.0));
    for (int a = 0; a < p; ++a) for (int b = 0; b < p; ++b) {
        double c = 0.0; for (size_t i = 0; i < d.values.size(); ++i) c += (d.values[i][a]-mean[a])*(d.values[i][b]-mean[b]);
        r[a][b] = (sd[a] == 0.0 || sd[b] == 0.0) ? (a == b ? 1.0 : 0.0) : c / ((n-1.0)*sd[a]*sd[b]);
    }
    return r;
}

bool invert(Matrix a, Matrix& inv) {
    int n = (int)a.size(); inv.assign(n, std::vector<double>(n, 0.0));
    for (int i=0;i<n;++i) inv[i][i]=1.0;
    for (int c=0;c<n;++c) {
        int pivot=c; for(int r=c+1;r<n;++r) if(std::fabs(a[r][c])>std::fabs(a[pivot][c])) pivot=r;
        if(std::fabs(a[pivot][c])<1e-12) return false;
        std::swap(a[pivot],a[c]); std::swap(inv[pivot],inv[c]); double q=a[c][c];
        for(int j=0;j<n;++j){a[c][j]/=q;inv[c][j]/=q;}
        for(int r=0;r<n;++r) if(r!=c){q=a[r][c];for(int j=0;j<n;++j){a[r][j]-=q*a[c][j];inv[r][j]-=q*inv[c][j];}}
    }
    return true;
}

bool independent(int x, int y, const Set& z, const Matrix& corr, int n, double alpha) {
    std::vector<int> vars; vars.push_back(x); vars.push_back(y); vars.insert(vars.end(), z.begin(), z.end());
    Matrix sub(vars.size(), std::vector<double>(vars.size()));
    for(size_t i=0;i<vars.size();++i)for(size_t j=0;j<vars.size();++j)sub[i][j]=corr[vars[i]][vars[j]];
    Matrix precision; if(!invert(sub,precision)) return false;
    double den=precision[0][0]*precision[1][1]; if(den<=0.0) return false;
    double r=-precision[0][1]/std::sqrt(den); r=std::max(-0.999999,std::min(0.999999,r));
    double dof = (double)n - (double)z.size() - 3.0; if(dof<=0.0) return false;
    double statistic=std::fabs(std::atanh(r))*std::sqrt(dof);
    double p=std::erfc(statistic/std::sqrt(2.0)); return p > alpha;
}

void combinations(const std::vector<int>& pool, int need, int start, Set& current,
                  std::vector<Set>& out) {
    if ((int)current.size()==need) { out.push_back(current); return; }
    for (int i=start; i <= (int)pool.size()-(need-(int)current.size()); ++i) {
        current.push_back(pool[i]); combinations(pool,need,i+1,current,out); current.pop_back();
    }
}
std::vector<Set> allSubsets(const std::vector<int>& pool, int maxSize) {
    std::vector<Set> out; Set cur; int top=std::min(maxSize,(int)pool.size());
    for(int k=0;k<=top;++k)combinations(pool,k,0,cur,out); return out;
}
std::string pairKey(int a,int b){if(a>b)std::swap(a,b);std::ostringstream s;s<<a<<":"<<b;return s.str();}
typedef std::map<std::string, Set> SepMap;

void prunePair(Graph& g, int x, int y, const std::vector<int>& pool, int maxSize,
               const Matrix& corr, int n, double alpha, SepMap& sep) {
    if(!g.adjacent(x,y))return;
    std::vector<Set> sets=allSubsets(pool,maxSize);
    for(size_t i=0;i<sets.size();++i) if(independent(x,y,sets[i],corr,n,alpha)) {
        g.remove(x,y); sep[pairKey(x,y)]=sets[i]; return;
    }
}

std::vector<int> possibleDsep(const Graph& g, int source, int target) {
    int n=(int)g.mark.size(); std::vector<int> result, parent(n,-1); std::vector<int> q(1,source); parent[source]=source;
    for(size_t qi=0;qi<q.size();++qi){int b=q[qi];
        for(int c=0;c<n;++c) if(g.adjacent(b,c)&&c!=parent[b]&&c!=source) {
            bool allowed=(parent[b]>=0 && g.adjacent(parent[b],c));
            if(parent[b]>=0 && g.adjacent(parent[b],b) && g.adjacent(b,c))
                allowed=allowed || (g.mark[b][parent[b]]==ARROW && g.mark[b][c]==ARROW);
            if(allowed && parent[c]==-1){parent[c]=b;q.push_back(c);if(c!=target)result.push_back(c);}
        }
    }
    std::sort(result.begin(),result.end()); result.erase(std::unique(result.begin(),result.end()),result.end()); return result;
}

bool isDirected(const Graph& g,int a,int b){return g.adjacent(a,b)&&g.mark[a][b]==TAIL&&g.mark[b][a]==ARROW;}
void orientCollider(Graph& g,int x,int y,int z){g.mark[y][x]=ARROW;g.mark[y][z]=ARROW;}

void orient(Graph& g,const SepMap& sep) {
    int n=(int)g.mark.size();
    // Unshielded colliders: orient x *-> y <-* z when y is absent from Sep(x,z).
    for(int y=0;y<n;++y){std::vector<int> ns=g.neighbors(y);for(size_t i=0;i<ns.size();++i)for(size_t j=i+1;j<ns.size();++j){
        int x=ns[i],z=ns[j];if(g.adjacent(x,z))continue;SepMap::const_iterator it=sep.find(pairKey(x,z));if(it==sep.end())continue;
        if(std::find(it->second.begin(),it->second.end(),y)==it->second.end())orientCollider(g,x,y,z);
    }}
    bool changed=true;int rounds=0;
    while(changed&&rounds++<n*n){changed=false;
        // R1: a *-> b o-* c and a,c nonadjacent => b -> c.
        for(int a=0;a<n;++a)for(int b=0;b<n;++b)if(a!=b&&g.adjacent(a,b)&&g.mark[b][a]==ARROW)
            for(int c=0;c<n;++c)if(c!=a&&c!=b&&g.adjacent(b,c)&&!g.adjacent(a,c)&&g.mark[b][c]==CIRCLE&&g.mark[c][b]==CIRCLE){g.mark[b][c]=TAIL;g.mark[c][b]=ARROW;changed=true;}
        // R2: a -> ... -> b and a o-* b => replace the circle at a with a tail.
        for(int a=0;a<n;++a)for(int b=0;b<n;++b)if(a!=b&&g.adjacent(a,b)&&g.mark[a][b]==CIRCLE&&g.mark[b][a]==ARROW){
            bool path=false;for(int m=0;m<n;++m)if(m!=a&&m!=b&&isDirected(g,a,m)&&isDirected(g,m,b))path=true;
            if(path){g.mark[a][b]=TAIL;changed=true;}
        }
    }
}

std::string endpoint(int m){return m==CIRCLE?"o":m==TAIL?"-":m==ARROW?">":"?";}
}

bool runFci(const std::string& csvPath, double alpha, int maxConditioningSize,
            FciPagResult& result, std::string& error) {
    if (alpha <= 0.0 || alpha >= 1.0) { error = "alpha must be between 0 and 1"; return false; }
    Data d;
    if (!readCsv(csvPath, d)) { error = "could not read numeric CSV with header and at least 5 rows"; return false; }
    const int p = (int)d.names.size(), n = (int)d.values.size();
    Matrix corr = correlations(d); Graph g(p); SepMap sep;
    int maxDepth = maxConditioningSize < 0 ? p - 2 : std::min(maxConditioningSize, p - 2);
    for (int depth = 0; depth <= maxDepth; ++depth) {
        bool any = false;
        for (int x = 0; x < p; ++x) for (int y = x + 1; y < p; ++y) if (g.adjacent(x, y)) {
            std::vector<int> pool = g.neighbors(x); pool.erase(std::remove(pool.begin(), pool.end(), y), pool.end());
            if ((int)pool.size() >= depth) { prunePair(g, x, y, pool, depth, corr, n, alpha, sep); any = true; }
            if (g.adjacent(x, y)) {
                pool = g.neighbors(y); pool.erase(std::remove(pool.begin(), pool.end(), x), pool.end());
                if ((int)pool.size() >= depth) { prunePair(g, x, y, pool, depth, corr, n, alpha, sep); any = true; }
            }
        }
        if (!any) break;
    }
    for (int x = 0; x < p; ++x) for (int y = x + 1; y < p; ++y) if (g.adjacent(x, y)) {
        std::vector<int> pool = possibleDsep(g, x, y), reverse = possibleDsep(g, y, x);
        pool.insert(pool.end(), reverse.begin(), reverse.end());
        std::sort(pool.begin(), pool.end()); pool.erase(std::unique(pool.begin(), pool.end()), pool.end());
        if (maxConditioningSize >= 0 && pool.size() > (size_t)maxConditioningSize) pool.resize(maxConditioningSize);
        prunePair(g, x, y, pool, maxConditioningSize < 0 ? (int)pool.size() : maxConditioningSize, corr, n, alpha, sep);
    }
    orient(g, sep);
    FciPagResult found; found.variables = d.names; found.observations = n;
    for (int i = 0; i < p; ++i) for (int j = i + 1; j < p; ++j) if (g.adjacent(i, j)) {
        FciPagEdge edge; edge.left = d.names[i]; edge.right = d.names[j];
        edge.leftMark = endpoint(g.mark[i][j])[0]; edge.rightMark = endpoint(g.mark[j][i])[0];
        found.edges.push_back(edge);
    }
    result = found;
    return true;
}

bool hasPagAdjacency(const FciPagResult& result, const std::string& a, const std::string& b) {
    for (size_t i = 0; i < result.edges.size(); ++i)
        if ((result.edges[i].left == a && result.edges[i].right == b) ||
            (result.edges[i].left == b && result.edges[i].right == a)) return true;
    return false;
}

bool hasDirectedPagEdge(const FciPagResult& result, const std::string& from, const std::string& to) {
    for (size_t i = 0; i < result.edges.size(); ++i) {
        const FciPagEdge& e = result.edges[i];
        if (e.left == from && e.right == to && e.leftMark == '-' && e.rightMark == '>') return true;
        if (e.left == to && e.right == from && e.leftMark == '>' && e.rightMark == '-') return true;
    }
    return false;
}
