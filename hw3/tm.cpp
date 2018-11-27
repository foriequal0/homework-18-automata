#include <iostream>
#include <string>
#include <unordered_map>
#include <cassert>
#include <iterator>
#include <vector>
#include <algorithm>
#include <cstdlib>

/* Enable to print the intermediate transition */
static bool debug_transition = false;

/* Possible alphabets */
static const char alphabets[] = "01#abcdefghijklmnopqrstuvwxyz";

/* Possible movements */
enum class Move {
  S, R, L
};

std::ostream& operator<<(std::ostream &os, const Move& move) {
  switch(move) {
  case Move::S: os << "S"; break;
  case Move::R: os << "R"; break;
  case Move::L: os << "L"; break;
  }
  return os;
}


int to_digit(std::string word) {
  return std::stoi(word);
}

Move to_move(std::string c) {
  if (c.compare("S") == 0) { return Move::S; }
  else if (c.compare("R") == 0) { return Move::R; }
  else if (c.compare("L") == 0) { return Move::L; }
  else { assert(false); }
}

char to_alphabet(std::string word, int count) {
  assert(word.size() == 1);
  auto c = word[0];
  switch(c) {
  case '0': case '1': case '#': return c;
  default:
    assert(c >= 'a' && c <= 'z');
    assert((c-'a') < count);
    return c;
  }
}

struct Transition {
  bool valid;
  int state_to;
  char write_to;
  Move move_to;

  Transition(): valid(false), state_to(0), write_to('\0'), move_to(Move::S) {}
  Transition(int state_to, char change_to, Move move_to)
    : valid(true), state_to(state_to), write_to(change_to), move_to(move_to)
  {}

  static Transition read(std::istream& is, int alphabet_count) {
    std::string a, b, c;
    is >> a >> b >> c;
    /* first one is -, then it is infinite loop */
    if (a.compare("-") == 0) {
      assert(b.compare("-") == 0 && c.compare("-") == 0);
      return Transition();
    }
    return Transition(to_digit(a),
                      to_alphabet(b, alphabet_count),
                      to_move(c));
  }
};

std::ostream& operator<<(std::ostream &os, const Transition& tr) {
  if (tr.valid) {
    os << tr.state_to << " " << tr.write_to << " " << tr.move_to;
  } else {
    os << "- - -";
  }
  return os;
}

struct Table {
  int K;
  std::vector<std::unordered_map<char, Transition>> state_transitions;
  std::vector<bool> halt;

  Table(int K, std::vector<std::unordered_map<char, Transition>> state_transitions, std::vector<bool> halt)
    : K(K), state_transitions(state_transitions), halt(halt) { }

  static Table read(std::istream &is) {
    int K, N;
    std::string halt_input;
    std::cin >> K >> N >> halt_input;

    std::vector<std::unordered_map<char, Transition>> state_transitions;
    std::vector<bool> halt;
    for(int i=0; i<N; i++) {
      std::unordered_map<char, Transition> transitions;
      for(int i=0; i < 3 + K; i++) {
        transitions[alphabets[i]] = Transition::read(is, K);
      }
      state_transitions.push_back(transitions);
    }
    std::transform(std::begin(halt_input), std::end(halt_input),
      std::back_inserter(halt), [&](auto c) {
        switch(c) {
        case '0': return false;
        case '1': return true;
        default: assert(false);
        }
      });
    assert(state_transitions.size() == halt.size());
    return Table(K, std::move(state_transitions), std::move(halt));
  }

  Transition get_transition(int state, char a) const {
    auto tr = state_transitions[state];
    auto it = tr.find(a);
    if (it != tr.end()) {
      return it->second;
    }
    return Transition();
  }

  bool is_halted(int state) const {
    return halt[state];
  }
};

std::ostream& operator<<(std::ostream &os, const Table& table) {
  os << table.K << std::endl;
  os << table.state_transitions.size() << std::endl;
  for(bool h: table.halt) {
    os << h;
  }
  os << std::endl;
  bool first = true;
  for(auto transitions: table.state_transitions) {
    for(char ch: alphabets) {
      auto it = transitions.find(ch);
      if (it == transitions.end()) {
        break;
      }

      if (first) { first = false; }
      else {
        os << std::endl;
      }

      auto tr = it->second;
      os << tr;
    }
  }
  return os;
}

struct Tape {
  int pos;
  int left_end;
  int right_end;
  std::unordered_map<int, char> tape;

  Tape(std::string input)
    : pos(-1), left_end(0), right_end(std::max((int)input.size()-1, 0)) {
    for(size_t i=0; i < input.size(); i++) {
      tape[i] = input[i];
    }
  }

  char get(int i) const {
    auto at = tape.find(i);
    if (at == tape.end()) {
      return '#';
    }
    return at->second;
  }

  char read() const {
    return get(pos);
  }

  void write(char c) {
    /* write non-empty char */
    if (c != '#') {
      tape[pos] = c;
      /* extends left_end, right_end bound */
      if (pos < left_end) {
        left_end = pos;
      } else if (pos > right_end) {
        right_end = pos;
      }
      return;
    }
    /* write empty-char */
    tape.erase(pos);
    /* contract left_end, right_end bound until it fits*/
    if (pos == left_end) {
      while(left_end < right_end && get(left_end) == '#') {
        left_end++;
      }
    } else if (pos == right_end) {
      while(left_end < right_end && get(right_end) == '#') {
        right_end--;
      }
    }
  }

  void move(Move dir) {
    switch(dir) {
    case Move::S: return;
    case Move::L: pos--; return;
    case Move::R: pos++; return;
    }
  }

  std::string as_string() const {
    std::string str;
    for(int i=left_end; i<=right_end; i++) {
      str.push_back(get(i));
    }
    return str;
  }
};

std::ostream& operator<<(std::ostream& os, const Tape& tape) {
  for(int i=std::min(tape.pos, tape.left_end); i<= std::max(tape.pos, tape.right_end); i++) {
    if (i == tape.left_end && i == tape.right_end) {
      os << "x";
    } else if (i == tape.left_end) {
      os << "l";
    } else if (i == tape.right_end) {
      os << "r";
    } else {
      os << " ";
    }
  }
  os << std::endl;
  for(int i=std::min(tape.pos, tape.left_end); i<= std::max(tape.pos, tape.right_end); i++) {
    os << tape.get(i);
  }
  os << std::endl;

  for(int i=std::min(tape.pos, tape.left_end); i<= std::max(tape.pos, tape.right_end); i++) {
    if (i == tape.pos) {
      os << "^";
    } else {
      os << " ";
    }
  }
  return os;
}

Tape run(const Table& table, Tape tape) {
  if (debug_transition) {
    std::cout << tape << std::endl;
  }

  int state = 0;
  while (table.is_halted(state) == false) {
    /* read tape */
    auto read = tape.read();

    /* read transitioon table */
    auto transition = table.get_transition(state, read);
    if (transition.valid == false) {
      /* it is infinite loop. exit with error */
      exit(-1);
    }

    /* write it, move header, update state */
    tape.write(transition.write_to);
    tape.move(transition.move_to);
    state = transition.state_to;

    if (debug_transition) {
      std::cout << transition << std::endl;
      std::cout << tape << std::endl;
    }
  }
  /* return final state tape */
  return tape;
}

int main() {
  if (std::getenv("DEBUG_TRANSITION") != nullptr) {
    debug_transition = true;
  }
  /* read table */
  auto table = Table::read(std::cin);
  /* initialize tape */
  std::string input;
  std::cin >> input;
  Tape tape { input };

  auto result = run(table, tape);
  /* print tape */
  std::cout << result.as_string() << std::endl;
}
