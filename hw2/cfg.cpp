#include "cfg.hpp"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <cassert>

parse_error::parse_error(const std::string& whatarg)
  : runtime_error(whatarg) {}

unexpected_eol::unexpected_eol()
  : parse_error("Unexpected EOL") {}

Symbol::Base::~Base() {}

Symbol::Var::Var(std::string s): value(s) {}

bool Symbol::Var::operator==(const Symbol::Var& other) const {
  return this->value == other.value;
}

bool Symbol::Var::operator!=(const Symbol::Var& other) const {
  return this->value != other.value;
}

bool Symbol::Var::operator<(const Symbol::Var& other) const {
  return this->value < other.value;
}

Symbol::Term::Term(char c): value(c) {}

bool Symbol::Term::operator==(const Symbol::Term& other) const {
  return this->value == other.value;
}

bool Symbol::Term::operator!=(const Symbol::Term& other) const {
  return this->value != other.value;
}

bool Symbol::Term::operator<(const Symbol::Term& other) const {
  return this->value < other.value;
}

Symbol::Symbol(Var v): value(std::shared_ptr<Var>(new Var(v))) { }

Symbol::Symbol(Term t): value(std::shared_ptr<Term>(new Term(t))) { }

bool Symbol::is_var() const {
  assert(this->value != nullptr);
  return dynamic_cast<Symbol::Var*>(this->value.get()) != nullptr;
}

bool Symbol::is_term() const {
  assert(this->value != nullptr);
  return dynamic_cast<Symbol::Term*>(this->value.get()) != nullptr;
}

const Symbol::Var& Symbol::as_var() const {
  assert(this->value != nullptr);
  return *dynamic_cast<Symbol::Var*>(this->value.get());
}

const Symbol::Term& Symbol::as_term() const {
  assert(this->value != nullptr);
  return *dynamic_cast<Symbol::Term*>(this->value.get());
}

Symbol Symbol::read(std::istream& is) {
  auto la = is.peek();
  if (la == EOF)
    throw unexpected_eol();
  if (std::isalpha(la))
    return Symbol::read_var(is);
  return Symbol::read_term(is);
}

Symbol::Var Symbol::read_var(std::istream& is) {
  int la = is.peek();
  if (la == EOF)
    throw unexpected_eol();
  if (!std::isalpha(la)) {
    throw parse_error("expected an alphabet");
  }
  auto c = is.get();
  if (c == EOF) throw unexpected_eol();
  return Symbol::Var(std::string(1, c));
}

Symbol::Term Symbol::read_term(std::istream& is) {
  auto la = is.peek();
  if (la == EOF)
    throw unexpected_eol();
  static const char specialchars[] = {'+', '-', '*', '/', '(', ')'};
  auto unexpected = std::find(std::begin(specialchars), std::end(specialchars), la) == std::end(specialchars);
  if (!std::isdigit(la) && unexpected) {
    throw parse_error("expected a special char or digits");
  }
  auto c = is.get();
  if (c == EOF) throw unexpected_eol();
  return Symbol::Term(c);
}

std::ostream& operator<<(std::ostream& os, const Symbol& symbol) {
  if (symbol.is_var()) {
    auto var = symbol.as_var();
      os << "<" << var.value << ">";
  } else {
    auto term = symbol.as_term();
    os << term.value;
  }
  return os;
}

bool Symbol::operator==(const Symbol& other) const {
  if (this->is_var() && other.is_var()) {
    return (this->as_var()) == (other.as_var());
  } else if (this->is_term() && other.is_term()) {
    return (this->as_term()) == (other.as_term());
  }
  return false;
}

bool Symbol::operator!=(const Symbol& other) const {
  return !(*this == other);
}

bool Symbol::operator<(const Symbol& other) const {
  if (this->is_var() && other.is_var()) {
    return (this->as_var()) < (other.as_var());
  } else if (this->is_term() && other.is_term()) {
    return (this->as_term()) < (other.as_term());
  } else if (this->is_term()) {
    return true;
  }
  return false;
}

Prod::Prod(Symbol::Var lhs, std::vector<Symbol> rhs): lhs(lhs), rhs(rhs){}

bool Prod::is_epsilon() const {
  return this->rhs.size() == 0;
}

bool Prod::is_unit() const {
  return this->rhs.size() == 1 && this->rhs[0].is_var();
}

bool Prod::operator==(const Prod& other) const {
  if (this->lhs != other.lhs) return false;
  if (this->rhs.size() != other.rhs.size()) return false;
  for (size_t i=0; i<this->rhs.size(); i++) {
    if (this->rhs[i] != other.rhs[i]) {
      return false;
    }
  }
  return true;
}

bool Prod::operator!=(const Prod& other) const {
  return !(*this == other);
}

bool Prod::operator<(const Prod& other) const {
  if (this->lhs < other.lhs) return true;
  else if (this->lhs == other.lhs) {
    for(size_t i=0; i < std::min(this->rhs.size(), other.rhs.size()); i++) {
      if (this->rhs[i] < other.rhs[i]) { return true; }
      else if (this->rhs[i] == other.rhs[i]) { continue; }
      else return false;
    }
    if (this->rhs.size() < other.rhs.size()) {
      return true;
    }
  }
  return false;
}

Prod Prod::read(std::istream& is) {
  auto lhs = Symbol::read_var(is);
  if (is.get() != ':') throw parse_error("expected ':'");
  std::vector<Symbol> rhs;
  while (is.peek() != EOF) {
    auto symbol = Symbol::read(is);
    rhs.push_back(symbol);
  }
  return Prod(lhs, rhs);
}

std::istream& safe_getline(std::istream& is, std::string& t)
{
  // https://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
  t.clear();

  // The characters in the stream are read one-by-one using a std::streambuf.
  // That is faster than reading them one-by-one using the std::istream.
  // Code that uses streambuf this way must be guarded by a sentry object.
  // The sentry object performs various tasks,
  // such as thread synchronization and updating the stream state.

  std::istream::sentry se(is, true);
  std::streambuf* sb = is.rdbuf();

  for(;;) {
    int c = sb->sbumpc();
    switch (c) {
    case '\n':
      return is;
    case '\r':
      if(sb->sgetc() == '\n')
        sb->sbumpc();
      return is;
    case std::streambuf::traits_type::eof():
      // Also handle the case when the last line has no line ending
      if(t.empty())
        is.setstate(std::ios::eofbit);
      return is;
    default:
      t += (char)c;
    }
  }
}

static Prod read_line(std::istream& is) {
  std::string line;
  do {
    safe_getline(is, line);
    if (is.bad()) throw unexpected_eol();
  } while (line.length() == 0);
  auto ss = std::istringstream(line);
  return Prod::read(ss);
}

std::ostream& operator<<(std::ostream& os, const Prod& prod) {
  auto lhs = Symbol(prod.lhs);
  os << lhs << ":";
  for (auto s : prod.rhs) {
    os << s;
  }
  return os;
}

CFG::CFG(Symbol::Var start, std::vector<Prod> prods)
  : start(start), prods(prods) { }

Symbol::Var CFG::new_id() {
  std::stringstream ss;
  ss << "C" << this->next_ids++;
  return Symbol::Var(ss.str());
}

CFG CFG::read(std::istream& is) {
  int N;
  is >> N;
  std::vector<Prod> prods;
  auto first_prod = read_line(is);
  prods.push_back(first_prod);
  for (int i = 0; i < N-1; i++) {
    auto prod = read_line(is);
    prods.push_back(prod);
  }
  return CFG(first_prod.lhs, prods);
}

std::ostream& operator<<(std::ostream& os, const CFG& cfg) {
  os << cfg.prods.size() << std::endl;
  for(auto prod: cfg.prods) {
    if (prod.lhs == cfg.start) {
      os << prod << std::endl;
    }
  }
  for(auto prod: cfg.prods) {
    if (prod.lhs == cfg.start) { continue; }
    os << prod << std::endl;
  }
  return os;
}
