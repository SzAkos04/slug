#pragma once

#include <string>
#include <variant>

class Literal {
  public:
    using Value = std::variant<int, unsigned int, double, std::string, bool>;

    Literal() = default;
    ~Literal() = default;

    explicit Literal(int v) : value(v) {}
    explicit Literal(unsigned int v) : value(v) {}
    explicit Literal(double v) : value(v) {}
    explicit Literal(const std::string &v) : value(v) {}
    explicit Literal(bool v) : value(v) {}

    Value get() const { return this->value; }

  private:
    Value value;
};
