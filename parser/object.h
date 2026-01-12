#pragma once

#include <memory>
#include <string>
#include <utility>

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
};

class Number : public Object {
public:
    Number(int value) : value_(value) {
    }
    int GetValue() const {
        return value_;
    }

private:
    int value_;
};

class Symbol : public Object {
public:
    Symbol(std::string ch) : ch_(ch) {
    }
    const std::string& GetName() const {
        return ch_;
    }

private:
    std::string ch_;
};

class Cell : public Object {
public:
    std::shared_ptr<Object> GetFirst() const {
        return cell_.first;
    }
    std::shared_ptr<Object> GetSecond() const {
        return cell_.second;
    }
    void SetFirst(std::shared_ptr<Object> first) {
        cell_.first = std::move(first);
    }
    void SetSecond(std::shared_ptr<Object> second) {
        cell_.second = std::move(second);
    }

private:
    std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>> cell_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and conversion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    return As<T>(obj) != nullptr;
}
