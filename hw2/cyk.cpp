#include "cfg.hpp"
#include "to_cnf.hpp"
#include <unordered_set>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cassert>

using namespace std;
#define RANGE(x) begin(x), end(x)

template<typename K, typename V>
using umap = unordered_map<K, V>;

template<typename T>
using uset = unordered_set<T>;

using Var = Symbol::Var;
using Term = Symbol::Term;

namespace std {
template<typename T, typename U>
struct hash<pair<T, U>> {
  size_t operator()(const pair<T, U>& pair) const {
    auto a = std::hash<T>()(pair.first);
    auto b = std::hash<U>()(pair.second);
    return (size_t) a * 16777619 + b;
  }
};
};

umap<char, uset<Var>> build_term_index(const CFG& cfg) {
  umap<char, uset<Var>> index;
  for(auto prod: cfg.prods) {
    if (prod.rhs.size() == 1 && prod.rhs[0].is_term()) {
      index[prod.rhs[0].as_term().value].insert(prod.lhs);
    }
  }
  return index;
}

umap<pair<Var, Var>, uset<Var>> build_var_index(const CFG& cfg) {
  umap<pair<Var, Var>, uset<Var>> index;
  for(auto prod: cfg.prods) {
    if (prod.rhs.size() == 2 && prod.rhs[0].is_var() && prod.rhs[1].is_var()) {
      auto a = prod.rhs[0].as_var();
      auto b = prod.rhs[1].as_var();
      index[{a, b}].insert(prod.lhs);
    }
  }
  return index;
}

void print_table(umap<pair<int, int>, uset<Var>> &V, int n){
  for(int j=0; j<n; j++) {
    std::cout << "\t" << j;
  }
  std::cout << std::endl;
  for(int i=0; i<n; i++) {
    std::cout << i;
    for(int j=0; j<n; j++) {
      std::cout << "\t";
      for(auto a:V[{i,j}]) {
        std::cout << a;
      }
    }
    std::cout << std::endl;
  }
}

bool cyk(const CFG& cfg, const std::string& a) {
  auto term_index = build_term_index(cfg);
  auto var_index = build_var_index(cfg);
  auto n = a.size();
  umap<pair<int, int>, uset<Var>> V;
  // for i = 1 to n do
  for (size_t i=0; i<n; i++) {
    // V_ii = { A: A->a_i }
    V[{i,i}] = term_index[a[i]];
  }
  // for d = 1 to n-1 do
  for (size_t d = 1; d <= n - 1; d++) {
    // for i = 1 to n - d do
    for(size_t i = 0; i < n - d; i++) {
      auto j = i + d;
      // for k = i to j - 1 do
      for (size_t k = i; k < j; k++) {
        // Vij = Vij U { A: A->BC, B in V_ik, C in V_(i+1)j }
        for(auto b: V[{i,k}]) {
          for(auto c: V[{k+1,j}]) {
            auto a = var_index[{b, c}];
            V[{i,j}].insert(RANGE(a));
          }
        }
      }
    }
  }
  // S in V_1n
  auto V1n = V[{0, n-1}];
  return V1n.find(cfg.start) != std::end(V1n);
}

bool is_cnf(const CFG& cfg) {
  for(auto prod: cfg.prods) {
    if (prod.rhs.size() == 1 && prod.rhs[0].is_term()) continue;
    if (prod.rhs.size() == 2 && prod.rhs[0].is_var() && prod.rhs[1].is_var()) continue;
    return false;
  }
  return true;
}

int main(int argc, char *argv[]) {
  const CFG cfg = CFG::read(std::cin);
  string str;
  cin >> str;
  bool parsed;
  if (argc > 1 && strcmp(argv[1], "+") == 0) {
    auto cnf = to_cnf(cfg);
    assert(is_cnf(cnf));
    parsed = cyk(cnf, str);
  } else {
    if (!is_cnf(cfg)) {
      throw runtime_error("out of homework specification. input must be a CNF");
    }
    parsed = cyk(cfg, str);
  }
  std::cout << (parsed?"Yes":"No");
}
