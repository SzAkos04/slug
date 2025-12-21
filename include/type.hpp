#pragma once

#include <ostream>
enum class PrimitiveType {
    Void,
    I32,
    F64,
    Bool,
    String,

    Unknown,
};

struct Type {
    PrimitiveType kind;

    Type() : kind(PrimitiveType::Unknown) {}
    explicit Type(PrimitiveType kind) : kind(kind) {}

    bool operator==(const Type &other) const { return kind == other.kind; }
    bool operator!=(const Type &other) const { return kind != other.kind; }
};

std::ostream &operator<<(std::ostream &os, PrimitiveType t);
