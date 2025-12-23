#pragma once

#include <variant>

class Literal {
  public:
    using Value = std::variant<int, double, bool>;

    Literal() = default;
    ~Literal() = default;

    explicit Literal(int v) : value(v) {}
    explicit Literal(double v) : value(v) {}
    explicit Literal(bool v) : value(v) {}

    Value get() const { return this->value; }

  private:
    Value value;
};
