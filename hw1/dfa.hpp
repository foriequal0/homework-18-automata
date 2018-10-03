#ifndef __DFA_HPP__
#define __DFA_HPP__

#include <string>
#include <tuple>
#include <vector>
#include <set>
#include <map>
#include <iostream>

struct DFA {
  using state_type = int;
  using alphabet_type = int;
  using transition_in_type = std::tuple<state_type, alphabet_type>;
  using transition_out_type = state_type;

  std::set<state_type> states;
  std::vector<alphabet_type> alphabets;
  std::map<transition_in_type, transition_out_type> transition_map;
  state_type initial_state;
  std::set<state_type> final_states;

  DFA(state_type initial_state, std::vector<alphabet_type> alphabets);
  DFA(const DFA&) = default;
  DFA(DFA&&) = default;
  DFA& operator=(const DFA&) = default;
  DFA& operator=(DFA&&) = default;

  void write(std::ostream &os) const;
};

#endif
