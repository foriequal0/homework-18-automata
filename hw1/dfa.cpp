#include <algorithm>

#include "dfa.hpp"

DFA::DFA(state_type initial, std::vector<alphabet_type> alphabets)
  : alphabets(alphabets),
    initial_state(initial)
{ }


void DFA::write(std::ostream & os) const {
  os << states.size() << std::endl;
  std::vector<state_type> sorted_states;
  std::copy(std::begin(states), std::end(states), std::back_inserter(sorted_states));
  std::sort(std::begin(sorted_states), std::end(sorted_states));
  for(auto state: sorted_states) {
    os << state;
    if (final_states.find(state) != std::end(final_states)) {
      os << " " << "1";
    } else {
      os << " " << "0";
    }

    for(auto alphabet: alphabets) {
      auto outs = transition_map.find(transition_in_type(state, alphabet));
      if (outs == std::end(transition_map)) {
        os << " " << "-";
      } else {
        os << " " << outs->second;
      }
    }
    os << std::endl;
  }
}
