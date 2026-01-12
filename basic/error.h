#pragma once

#include <stdexcept>
#include <format>
struct SyntaxError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct RuntimeError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct NameError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct InvalidArgsCount : public RuntimeError {
    InvalidArgsCount() : RuntimeError("invalid arguments count") {
    }
    InvalidArgsCount(const std::string& details)
        : RuntimeError("invalid arguments count: " + details) {
    }
};
