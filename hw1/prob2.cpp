#include <iostream>
#include <vector>

#include "nfa.hpp"

int main() {
  auto nfa = NFA::read(std::cin, 0, {0, 1, NFA::epsilon});

  int N;
  std::cin >> N;
  for(int i=0; i<N; i++) {
    std::vector<NFA::alphabet_type> str;
    std::string line;
    std::cin >> line;
    for(char c:line) {
      if (c == '0')
        str.push_back(0);
      else if (c == '1')
        str.push_back(1);
    }

    auto result = nfa.run(str);
    if (result) {
      std::cout << "Yes" << std::endl;
    } else {
      std::cout << "No" << std::endl;
    }
  }
}
