#include <cassert>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <queue>

#include "nfa.hpp"

#define RANGE(x) std::begin(x), std::end(x)

const NFA::alphabet_type NFA::epsilon = -1;

NFA::NFA(state_type initial, std::vector<alphabet_type> alphabets)
  : alphabets(alphabets),
    initial_state(initial)
{ }

/* Read NFA from stream with given initial state and alphabets */
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

      // split by ,
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

/* Write NFA to stream */
void NFA::write(std::ostream & os) const {
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

/* Returns a set of all states that can be reached by transitions using epsilon from a state */
std::set<NFA::state_type> NFA::E(const state_type& start_state) const {
  std::set<NFA::state_type> result;
  result.insert(start_state);
  std::queue<NFA::state_type> to_visit;
  to_visit.push(start_state);
  while(to_visit.empty() == false) {
    NFA::state_type state = to_visit.front();
    to_visit.pop();

    for(auto epsilon_neighbor: transition(state, NFA::epsilon)) {
      if (result.find(epsilon_neighbor) == std::end(result)) {
        result.insert(epsilon_neighbor);
        to_visit.push(epsilon_neighbor);
      }
    }
  }
  return result;
}

/* Returns a set of all states that can be reached by transitions using epsilon from a set of all states */
std::set<NFA::state_type> NFA::E(const std::set<state_type>& states) const {
  std::set<NFA::state_type> result;
  for(auto state: states) {
    auto nexts = E(state);
    result.insert(RANGE(nexts));
  }
  return result;
}

/* Transition by a alphabet from a state */
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

/* Transition by a alphabet from a set of states */
std::set<NFA::state_type> NFA::transition(const std::set<state_type>& states,
                                          const alphabet_type& alphabet) const
{
  std::set<NFA::state_type> result;
  for(auto state: states) {
    auto nexts = transition(state, alphabet);
    result.insert(RANGE(nexts));
  }
  return result;
}

/* Transition by a alphabet from a set of states */
std::set<NFA::state_type> NFA::transitions
(const state_type& state,
 const std::vector<alphabet_type>& str) const
{
  auto states = E(state);
  for(auto x: str) {
    states = E(transition(states, x));
  }
  return states;
}


/* Helper type to use a set as a key for the unordered_map */
template<typename T>
struct set_hash {
  size_t operator()(const std::set<T> &x) const {
    /* variant of Fowler-Noll-Vo hash function
     * Took from https://en.cppreference.com/w/cpp/utility/hash/operator()
     */
    size_t result = 2166136261;

    for (auto v: x) {
      result = (result * 16777619) ^ std::hash<decltype(v)>()(v);
    }
    return result;
  }
};

/* Returns a unique DFA state from a set of NFA states */
class StateNameAllocator {
  std::unordered_map<std::set<NFA::state_type>, DFA::state_type, set_hash<NFA::state_type>> registered;
  DFA::state_type next_state = 0;
public:
  DFA::state_type get(const std::set<NFA::state_type>& states) {
    auto existing = registered.find(states);
    if (existing != std::end(registered)) {
      return existing->second;

    }
    auto state = next_state;
    next_state++;
    registered[states] = state;
    return state;
  }
};

/* Retuns whetehr a state is an accepted state */
bool NFA::is_accepted(const state_type& state) const {
  return final_states.find(state) != std::end(final_states);
}

/* Returns whether a set of state is also a final state */
bool NFA::is_accepted(const std::set<state_type>& states) const {
  std::vector<NFA::state_type> intersection;
  std::set_intersection(RANGE(final_states), RANGE(states),
                        std::back_inserter(intersection));
  // state contains any final state in NFA, so can be accepted. */
  return intersection.empty() == false;
}

/* Converts NFA to DFA.
   Nearly direct transition of psuedo-code of the textbook
   ----
   Qd <- { E(q0) }
   mark E(q0)
   while exists marked state P in Qd do
     unmark P
     for each a in alphabets do
       R <- E(Delta(P, a))
       if R is not in Qd then
         add R as marked state to Qd fi
       delta(P, a) <- R
     od
   od
*/
DFA NFA::into_dfa() const {
  StateNameAllocator alloc;

  // E(q0)
  auto initial_nfa_state = E(initial_state);
  auto initial_dfa_state = alloc.get(initial_nfa_state);

  /* DFA alphabets are NFA alphabets except an epsilon */
  std::vector<DFA::alphabet_type> dfa_alphabets;
  std::copy_if(RANGE(alphabets), std::back_inserter(dfa_alphabets),
               [](const auto &a) { return a != NFA::epsilon; });
  std::map<DFA::transition_in_type, DFA::transition_out_type> dfa_transition_map;
  std::set<DFA::state_type> dfa_states;
  std::set<DFA::state_type> dfa_final_states;
  dfa_states.insert(initial_dfa_state); // Qd <- { E(q0) }
  if (is_accepted(initial_nfa_state)) {
    dfa_final_states.insert(initial_dfa_state);
  }

  std::queue<std::set<NFA::state_type>> to_visit;
  to_visit.push(initial_nfa_state); // mark E(q0)
  /* while exists marked state P in Qd do */
  while(to_visit.empty() == false) {
    std::set<NFA::state_type> nfa_states = to_visit.front();
    to_visit.pop(); // unmark P
    auto dfa_state = alloc.get(nfa_states);

    /* for each a in alphabets do */
    for(auto alphabet: dfa_alphabets) {
      /* R <- E(Delta(P, a)) */
      auto next_nfa_state = E(transition(nfa_states, alphabet));
      auto next_dfa_state = alloc.get(next_nfa_state);
      /* delta(P, a) <- R */
      auto transition_in = DFA::transition_in_type(dfa_state, alphabet);
      dfa_transition_map[transition_in] = next_dfa_state;
      /* if R is not in Qd then */
      if (dfa_states.find(next_dfa_state) == std::end(dfa_states)) {
        /* add R to Qd */
        dfa_states.insert(next_dfa_state);
        if (is_accepted(next_nfa_state)) {
          dfa_final_states.insert(next_dfa_state);
        }
        /* R as marked state */
        to_visit.push(next_nfa_state);
      }
    }
  }

  DFA dfa(initial_dfa_state, dfa_alphabets);
  dfa.states = dfa_states;
  dfa.transition_map = dfa_transition_map;
  dfa.final_states = dfa_final_states;
  return dfa;
}

bool NFA::run(const std::vector<alphabet_type>& str) const {
  return is_accepted(transitions(initial_state, str));
}
