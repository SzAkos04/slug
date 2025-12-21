#include "type.hpp"

std::ostream &operator<<(std::ostream &os, PrimitiveType t) {
    switch (t) {
    case PrimitiveType::Void:
        return os << "Void";
    case PrimitiveType::I32:
        return os << "I32";
    case PrimitiveType::F64:
        return os << "F64";
    case PrimitiveType::Bool:
        return os << "Bool";
    case PrimitiveType::String:
        return os << "String";
    case PrimitiveType::Unknown:
        return os << "Unknown";
    }
    return os;
}
