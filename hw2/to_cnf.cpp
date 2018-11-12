#include "to_cnf.hpp"
#include "cfg.hpp"

#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <queue>
#include <cassert>
#include <sstream>
#include <stdexcept>

using namespace std;
using Var = Symbol::Var;
using Term = Symbol::Term;
#define RANGE(x) begin(x), end(x)

/// Implements 1.1
static unordered_set<Var> get_Ve(const CFG& cfg) {
  unordered_set<Var> Ve;
  bool changed;
  do {
    changed = false;
    for(auto prod: cfg.prods) {
      // If all variable in the rhs is an epsilon production (a member of Ve),
      auto epsilon_prod = all_of(RANGE(prod.rhs), [&](auto sym) {
          if (sym.is_var() == false) return false;
          return Ve.find(sym.as_var()) != end(Ve);
        });
      // then insert it to Ve
      if (epsilon_prod) {
        auto result = Ve.insert(prod.lhs);
        if (result.second) {
          changed = true;
        }
      }
    }
  } while (changed);
  return Ve;
}

static vector<bool> init_combination(const unordered_set<Var>& Ve, const vector<Symbol>& rhs) {
  auto size = count_if(RANGE(rhs), [&](auto sym) {
      if (sym.is_term()) return false;
      return Ve.find(sym.as_var()) != end(Ve);
    });
  vector<bool> arr;
  for(decltype(size) i=0; i<size; i++)
    arr.push_back(true);
  return arr;
}

static bool next_combination(vector<bool> &comb) {
  for (size_t i=0; i<comb.size(); i++) {
    if (comb[i]) { comb[i] = false; return true; }
    else { comb[i] = true; }
  }
  return false;
}

struct epsilon_combinator {
  const unordered_set<Var>& Ve;
  const vector<bool>& comb;
  epsilon_combinator(const unordered_set<Var>& Ve,
                      const vector<bool>& comb)
    : Ve(Ve), comb(comb) {}

  vector<Symbol> get(const vector<Symbol>& rhs) {
    vector<bool> current_combination = comb;
    vector<Symbol> new_rhs;
    for (auto sym: rhs) {
      if (sym.is_term()) { new_rhs.push_back(sym); continue; }
      auto Ve_member = Ve.find(sym.as_var()) != end(Ve);
      if (!Ve_member){ new_rhs.push_back(sym); continue; }
      if (current_combination.back()) {
        new_rhs.push_back(sym);
      }
      current_combination.pop_back();
    }
    return new_rhs;
  }
};

/// Implements 1
static CFG remove_epsilon(const CFG& G) {
  // 1.1
  auto Ve = get_Ve(G);

  unordered_set<Prod> P1_set;
  // 1.2 for each production rule in P,
  // insert every combination of an epsilon replaced rule
  for(auto prod: G.prods) {
    auto combination = init_combination(Ve, prod.rhs);
    do {
      auto rhs = epsilon_combinator(Ve, combination).get(prod.rhs);
      if (rhs.size() > 0) P1_set.insert(Prod { prod.lhs, rhs });
    } while (next_combination(combination));
  }

  // 1.3 G1 is equivalent with G
  auto G1 = CFG { G.start, {} };
  copy(RANGE(P1_set), back_inserter(G1.prods));
  sort(RANGE(G1.prods));
  return G1;
}

namespace std {
template<>
struct hash<pair<Var, Var>> {
  size_t operator()(const pair<Var, Var>& x) const {
    return hash<Var>()(x.first) * 31 + hash<Var>()(x.second);
  }
};
}

static vector<pair<Var, Var>>
get_unit_paths(const unordered_multimap<Var, vector<Symbol>>& prods) {
  unordered_set<Var> vars;
  for(auto kv: prods) {
    vars.insert(kv.first);
  }
  // DFS traversal to find unit derivatives from every variable
  unordered_set<pair<Var, Var>> unit_paths;
  for(auto var: vars) {
    unordered_set<Var> visited;
    queue<Var> queue;
    queue.push(var);
    while(!queue.empty()) {
      Var popped = queue.front();
      queue.pop();
      visited.insert(popped);
      auto adjs = prods.equal_range(popped);
      for_each(adjs.first, adjs.second, [&](auto pair) {
          auto prod = Prod { pair.first, pair.second };
          if (!prod.is_unit()) return;
          auto rhs = prod.rhs[0].as_var();
          if (visited.find(rhs) != std::end(visited)) return;
          queue.push(rhs);
          unit_paths.insert({var, rhs});
        });
    }
  }

  vector<pair<Var, Var>> result;
  copy(RANGE(unit_paths), back_inserter(result));
  sort(RANGE(result));
  return result;
}

/// Implements 2
static CFG remove_unit_paths(const CFG& G1) {
  unordered_multimap<Var, vector<Symbol>> P1;
  for(auto prod: G1.prods) {
    P1.insert({prod.lhs, prod.rhs});
  }

  // 2.1 Get unit paths that satisfy A =>* B
  auto unit_paths = get_unit_paths(P1);

  // 2.2 Insert productions that are not an unit production.
  unordered_set<Prod> P2_set;
  for(auto prod: G1.prods) {
    if (!prod.is_unit()) P2_set.insert(prod);
  }

  // 2.3 If A =>* B, B -> x in P1, and B -> is not an unit,
  // then insert A -> x to P2
  for(auto unit_path: unit_paths) {
    auto A = unit_path.first;
    auto B_xs = P1.equal_range(unit_path.second);
    for_each(B_xs.first, B_xs.second, [&](auto B_x) {
        auto B = B_x.first;
        auto x = B_x.second;
        assert(unit_path.second == B);
        auto reduced = Prod { A, x };
        if (!reduced.is_unit()) P2_set.insert(reduced);
      });
  }
  // 2.4 G2 == G1
  CFG G2 { G1.start, {} };
  copy(RANGE(P2_set), back_inserter(G2.prods));
  sort(RANGE(G2.prods));
  return G2;
}

static Prod term_to_prod(Symbol::Term term) {
  string name;
  if (isdigit(term.value)) {
    stringstream ss;
    ss << "A" << (char)term.value;
    name = ss.str();
  } else {
    switch(term.value) {
    case '+': name = "B0"; break;
    case '-': name = "B1"; break;
    case '*': name = "B2"; break;
    case '/': name = "B3"; break;
    case '(': name = "B4"; break;
    case ')': name = "B5"; break;
    default: throw runtime_error("out of homework specification");
    }
  }
  return Prod { Symbol::Var(name), { term } };
}

// Implements 3
CFG to_cnf(const CFG& cfg) {
  auto G1 = remove_epsilon(cfg);
  auto G2 = remove_unit_paths(G1);

  CFG G3 { G2.start, {} };
  std::unordered_set<Prod> P3;

  for(auto prod: G2.prods) {
    // 3.1 Insert A -> a into P3
    if (prod.rhs.size() == 1) {
      P3.insert(prod);
      continue;
    }
    // 3.2 Transform terminal to Ca -> a
    vector<Symbol> rhs;
    transform(RANGE(prod.rhs), back_inserter(rhs), [&](auto sym) {
        if (sym.is_var()) return sym;
        auto prod = term_to_prod(sym.as_term());
        P3.insert(prod);
        return Symbol(prod.lhs);
      });
    // 3.3 Intermediate variable and normalizing
    auto lhs = prod.lhs;
    for(size_t i=rhs.size() -1; i >= 2; i--) {
      auto next_lhs = G3.new_id();
      auto prod = Prod { lhs, { next_lhs, rhs[i] }};
      P3.insert(prod);
      lhs = next_lhs;
    }
    P3.insert({lhs, { rhs[0], rhs[1]}});
  }
  // 3.4 G3 == G2
  copy(RANGE(P3), back_inserter(G3.prods));
  sort(RANGE(G3.prods));
  return G3;
}
