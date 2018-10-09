#include <algorithm>

#include "dfa.hpp"

DFA::DFA(state_type initial, std::vector<alphabet_type> alphabets)
  : alphabets(alphabets),
    initial_state(initial)
{ }

/* Write DFA into stream */
void DFA::write(std::ostream & os) const {
  os << states.size() << std::endl;
  for(auto state: states) {
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
