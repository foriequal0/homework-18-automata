#include <cassert>
#include <sstream>
#include <algorithm>
#include <queue>

#include "nfa.hpp"

const NFA::alphabet_type NFA::epsilon = -1;

NFA::NFA(state_type initial, std::vector<alphabet_type> alphabets)
  : alphabets(alphabets),
    initial_state(initial)
{ }

NFA NFA::read(std::istream& is,
              const state_type initial,
              const std::vector<alphabet_type> alphabets)
{
  NFA nfa(initial, alphabets);
  int num_states;
  is >> num_states;
  assert(num_states > 0);
  for (auto i=0; i<num_states; i++) {
    state_type state;
    bool is_final;

    is >> state >> is_final;
    nfa.states.insert(state);
    if (is_final)
      nfa.final_states.insert(state);

    for(auto alphabet: nfa.alphabets) {
      std::string unparsed;
      is >> unparsed;
      if (unparsed == "-")
        continue;

      std::stringstream ss(unparsed);
      std::string out;
      while(std::getline(ss, out, ',')) {
        auto st = std::stoi(out);
        nfa.transition_map[transition_in_type(state, alphabet)].push_back(st);
      }
    }
  }
  return nfa;
}

void NFA::write(std::ostream & os) const {
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
        os << " ";
        int count=0;
        for(auto out:outs->second) {
          if (count == 0) os << out;
          else os << "," << out;
          count++;
        }
      }
    }
    os << std::endl;
  }
}

std::set<NFA::state_type> NFA::transition(const state_type& state,
                                          const alphabet_type& alphabet) const
{
  std::set<NFA::state_type> result;
  auto outs = transition_map.find(transition_in_type(state, alphabet));
  if (outs == std::end(transition_map))
    return result;

  for(auto out: outs->second) {
    result.insert(out);
  }
  return result;
}

std::set<NFA::state_type> NFA::transition(const std::set<state_type>& states,
                                     const alphabet_type& alphabet) const
{
  std::set<NFA::state_type> result;
  for(auto state: states) {
    auto nexts = transition(state, alphabet);
    result.insert(std::begin(nexts), std::end(nexts));
  }
  return result;
}

std::set<NFA::state_type> NFA::E(const state_type& start_state) const {
  std::set<NFA::state_type> res;
  res.insert(start_state);
  std::set<NFA::state_type> visited;
  std::queue<NFA::state_type> to_visit;
  to_visit.push(start_state);
  while(to_visit.empty() == false) {
    NFA::state_type state = to_visit.front();
    visited.insert(state);
    to_visit.pop();

    for(auto epsilon_neighbor: transition(state, NFA::epsilon)) {
      if (visited.find(epsilon_neighbor) != std::end(visited)) {
        continue;
      }
      res.insert(epsilon_neighbor);
      to_visit.push(epsilon_neighbor);
    }
  }
  return res;
}

std::set<NFA::state_type> NFA::E(const std::set<state_type>& states) const {
  std::set<NFA::state_type> result;
  for(auto state: states) {
    auto nexts = E(state);
    result.insert(std::begin(nexts), std::end(nexts));
  }
  return result;
}

DFA NFA::into_dfa() const {
  class StateNameAllocator {
    std::vector<std::set<NFA::state_type>> registered;
  public:
    DFA::state_type get(const std::set<NFA::state_type>& states) {
      int i=0;
      for(auto x: registered) {
        if (x == states) return DFA::state_type(i);
        i++;
      }
      registered.push_back(states);
      return i;
    }
  };

  StateNameAllocator alloc;
  auto initial_nfa_state = E(initial_state);
  auto initial_dfa_state = alloc.get(initial_nfa_state);
  std::vector<DFA::alphabet_type> dfa_alphabets;
  std::copy_if(std::begin(alphabets), std::end(alphabets),
               std::back_inserter(dfa_alphabets),
               [](auto &a) { return NFA::epsilon != a; });
  std::map<DFA::transition_in_type, DFA::transition_out_type> dfa_transition_map;
  std::set<DFA::state_type> dfa_final_states;
  std::set<DFA::state_type> dfa_states;
  std::queue<std::set<NFA::state_type>> to_visit;
  to_visit.push(initial_nfa_state);

  while(to_visit.empty() == false) {
    std::set<NFA::state_type> nfa_states = to_visit.front();
    to_visit.pop();
    auto dfa_state = alloc.get(nfa_states);
    dfa_states.insert(dfa_state);

    for(auto alphabet: dfa_alphabets) {
      auto next_nfa_states = E(transition(nfa_states, alphabet));
      auto next_dfa_state = alloc.get(next_nfa_states);

      dfa_transition_map[DFA::transition_in_type(dfa_state, alphabet)] = next_dfa_state;

      auto contains_any_final =
        std::any_of(std::begin(final_states), std::end(final_states),
                    [&](auto& f) { return next_nfa_states.find(f)
                                   != std::end(next_nfa_states); });

      if (contains_any_final) {
        dfa_final_states.insert(next_dfa_state);
      }
      if (dfa_states.find(next_dfa_state) != std::end(dfa_states)) {
        continue;
      }
      to_visit.push(next_nfa_states);
    }
  }

  DFA dfa(initial_dfa_state, dfa_alphabets);
  dfa.states = dfa_states;
  dfa.transition_map = dfa_transition_map;
  dfa.final_states = dfa_final_states;
  return dfa;
}

bool NFA::run(std::vector<alphabet_type> str) const {
  std::set<NFA::state_type> state = E(initial_state);
  for(auto x: str) {
    state = E(transition(state, x));
  }
  return std::any_of(std::begin(final_states), std::end(final_states),
                     [&](auto &f){ return state.find(f) != std::end(state); });
}
