#ifndef _CFG_H_
#define _CFG_H_

#include <string>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <unordered_map>

class parse_error : public std::runtime_error {
public:
  explicit parse_error (const std::string& whatarg);
};

class unexpected_eol : public parse_error {
public:
  explicit unexpected_eol();
};

#define DEF_CTR_ASSIGN(name) \
  name(const name&) = default; \
  name(name&&) = default; \
  name& operator=(const name&) = default;\
  name& operator=(name&&) = default

struct Symbol {
  struct Base { virtual ~Base(); };
public:
  struct Var : public Base {
    std::string value;
    Var(std::string);
    DEF_CTR_ASSIGN(Var);
    bool operator==(const Var&) const;
    bool operator!=(const Var&) const;
    bool operator<(const Var&) const;
  };
  struct Term : public Base {
    char value;
    Term(char);
    DEF_CTR_ASSIGN(Term);
    bool operator==(const Term&) const;
    bool operator!=(const Term&) const;
    bool operator<(const Term&) const;
  };

  std::shared_ptr<Base> value;

  Symbol(Var);
  Symbol(Term);
  DEF_CTR_ASSIGN(Symbol);

  bool is_var() const;
  bool is_term() const;

  const Symbol::Var& as_var() const;
  const Symbol::Term& as_term() const;

  static Symbol read(std::istream&);
  static Symbol::Var read_var(std::istream&);
  static Symbol::Term read_term(std::istream&);
  friend std::ostream& operator<<(std::ostream&, const Symbol&);

  bool operator==(const Symbol&) const;
  bool operator!=(const Symbol&) const;
  bool operator<(const Symbol&) const;
};

std::ostream& operator<<(std::ostream&, const Symbol&);

namespace std {
template<> struct hash<Symbol::Var> {
  size_t operator()(const Symbol::Var &var) const {
    return std::hash<std::string>()(var.value);
  }
};

template<> struct hash<Symbol::Term> {
  size_t operator()(const Symbol::Term &term) const {
    return std::hash<char>()(term.value);
  }
};

template<> struct hash<Symbol> {
  size_t operator()(const Symbol &sym) const {
    if (sym.is_term())
      return std::hash<Symbol::Term>()(sym.as_term());
    else
      return std::hash<Symbol::Var>()(sym.as_var());
  }
};
};
struct Prod {
  Symbol::Var lhs;
  std::vector<Symbol> rhs;

  Prod(Symbol::Var, std::vector<Symbol> rhs);
  DEF_CTR_ASSIGN(Prod);

  bool is_epsilon() const;
  bool is_unit() const;
  
  bool operator==(const Prod&) const;
  bool operator!=(const Prod&) const;
  bool operator<(const Prod&) const;

  static Prod read(std::istream&);
  friend std::ostream& operator<<(std::ostream&, const Prod&);
};

std::ostream& operator<<(std::ostream&, const Prod&);
namespace std {
template<> struct hash<Prod> {
  size_t operator()(const Prod &prod) const {
    auto h = std::hash<Symbol::Var>()(prod.lhs);
    for (auto s: prod.rhs) {
      h = h * 31 + std::hash<Symbol>()(s);
    }
    return h;
  }
};
}

struct CFG {
  Symbol::Var start;
  std::vector<Prod> prods;
  int next_ids = 0;

  CFG(Symbol::Var, std::vector<Prod>);
  DEF_CTR_ASSIGN(CFG);

  Symbol::Var new_id();

  static CFG read(std::istream&);
  friend std::ostream& operator<<(std::ostream&, const CFG&);
};

std::ostream& operator<<(std::ostream&, const CFG&);
#endif
