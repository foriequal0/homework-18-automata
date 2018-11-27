#include <unordered_map>
#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>

struct row { char write; char move; std::string transition; };

char move(char a) {
  switch(a) {
  case 's': case 'l': case 'r': return a - 'a' + 'A';
  case 'S': case 'L': case 'R': return a;
  default: assert(false);
  }
}

char lower(char a) {
  if (a >= 'A' && a <= 'Z'){ return a - 'A' + 'a'; }
  if (a >= 'a' && a <= 'z') { return a; }
  if (a == '0' || a == '1' || a == '#') { return a; }
  assert(false);
}

void update_max(char a, int&x) {
  if (a >= 'a' && a <= 'z') {
    x = std::max(x, a - 'a' + 1);
  }
}

int main() {
  std::unordered_map<std::string, std::unordered_map<char, row>> rows;
  std::unordered_map<std::string, int> label_state_map;
  std::vector<std::string> labels;
  std::vector<bool> halt = { };
  std::string label;
  int state=0;
  int max_alnum = 0;
  while(!std::cin.eof()) {
    char a, b, c;
    std::cin >> a;
    if (a == '-' || a == 'h') {
      std::cin >> label;
      assert(label_state_map.find(label) == label_state_map.end());
      label_state_map[label] = state++;
      labels.push_back(label);
      halt.push_back(a == 'h');
      continue;
    }
    if (a == '=') {
      break;
    }
    a = lower(a);
    std::string label_to;
    std::cin >> b >> c >> label_to;
    row r;
    r.write = lower(b); 
    r.move = move(c);
    r.transition = label_to;
    rows[label][a] = r;
    update_max(a, max_alnum);
    update_max(b, max_alnum);
  }
  std::cout << max_alnum << std::endl;
  std::cout << (labels.size()) << std::endl;
  for(auto b: halt) {
    std::cout << b;
  }
  std::cout << std::endl;
  const char alphabets[] = "01#abcdefghijklmnopqrstuvwxyz";
  for(int i=0; i< labels.size(); i++) {
    for(int j=0; j< 3+max_alnum; j++) {
      auto it = rows[labels[i]].find(alphabets[j]);
      if (it != rows[labels[i]].end()) {
        auto row = it->second;
        std::cout << label_state_map[row.transition] <<" " << row.write<<" " << row.move << std::endl;
      } else {
        std::cout << "- - -" << std::endl;;
      }
    }
  }
}
