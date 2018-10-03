#ifndef __NFA_HPP__
#define __NFA_HPP__

#include <string>
#include <tuple>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include "dfa.hpp"

struct NFA {
  using state_type = int;
  using alphabet_type = int;
  using transition_in_type = std::tuple<state_type, alphabet_type>;
  using transition_out_type = std::vector<state_type>;

  static const alphabet_type epsilon;

  std::set<state_type> states;
  std::vector<alphabet_type> alphabets;
  std::map<transition_in_type, transition_out_type> transition_map;
  state_type initial_state;
  std::set<state_type> final_states;

  NFA(state_type initial_state, std::vector<alphabet_type> alphabets);
  NFA(const NFA&) = default;
  NFA(NFA&&) = default;
  NFA& operator=(const NFA&) = default;
  NFA& operator=(NFA&&) = default;
  ~NFA() = default;

  static NFA read(std::istream& is,
                  const state_type initial,
                  const std::vector<alphabet_type> alphabets);

  void write(std::ostream& os) const;

  std::set<NFA::state_type> transition(const state_type& state,
                                       const alphabet_type& alphabet) const;
  std::set<NFA::state_type> transition(const std::set<state_type>& state,
                                       const alphabet_type& alphabet) const;
  std::set<NFA::state_type> E(const state_type& state) const;
  std::set<NFA::state_type> E(const std::set<state_type>& states) const;

  DFA into_dfa() const;
  bool run(std::vector<alphabet_type> string) const;
};

#endif
