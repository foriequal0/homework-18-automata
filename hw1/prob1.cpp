#include <cassert>
#include <iostream>
#include "nfa.hpp"
#include "dfa.hpp"

int main() {
  auto nfa = NFA::read(std::cin, 0, {0, 1, NFA::epsilon});
  auto dfa = nfa.into_dfa();
  dfa.write(std::cout);
}
