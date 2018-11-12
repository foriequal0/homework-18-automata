#include "cfg.hpp"
#include "to_cnf.hpp"
#include <iostream>

using namespace std;

int main() {
  auto cfg = CFG::read(cin);
  cout << to_cnf(cfg);
}
