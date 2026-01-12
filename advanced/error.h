#pragma once

#include <stdexcept>
#include <string>

struct SyntaxError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct RuntimeError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct InvalidArgsCount : public RuntimeError {
    template <typename... Args>
    InvalidArgsCount(Args&&... args)
        : RuntimeError("invalid arguments count: " + std::string(std::forward<Args>(args)...)) {
    }
};

struct NameError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};