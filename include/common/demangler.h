#ifndef HIGH_SHIFT_DEMANGLER_H
#define HIGH_SHIFT_DEMANGLER_H

#include <string>
#include <typeinfo>

namespace detail {
    std::string demangle(const char *name);
}

template<class T>
std::string demangled_type_name() {
    return detail::demangle(typeid(T).name());
}

#endif //HIGH_SHIFT_DEMANGLER_H
