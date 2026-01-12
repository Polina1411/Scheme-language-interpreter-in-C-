#pragma once

#include <memory>
#include <string>
#include <utility>
#include <array>
#include <vector>

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
};
using IntType = int64_t;
class Number : public Object {
public:
    Number(IntType value) : value_(value) {
    }
    IntType GetValue() const {
        return value_;
    }

private:
    IntType value_;
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

static const Symbol kSTrue{"#t"};
static const Symbol kSFalse{"#f"};
static const std::array<Symbol, 2> kBoolConsts = {kSTrue, kSFalse};
Symbol ToBoolSymbol(bool arg);
bool ToBool(const Symbol& symbol);
bool ToBool(const std::shared_ptr<Object>& object);
class IFunction : public Object {
public:
    std::vector<std::shared_ptr<Object>> Call(const std::vector<std::shared_ptr<Object>>&);

protected:
    virtual std::vector<std::shared_ptr<Object>> Prepare(
        const std::vector<std::shared_ptr<Object>>&) = 0;
    virtual std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>&) = 0;
};

class EvaluatingArgumentFunction : public IFunction {
    std::vector<std::shared_ptr<Object>> Prepare(
        const std::vector<std::shared_ptr<Object>>&) override;
};

class UnevaluatingArgumentFunction : public IFunction {
    std::vector<std::shared_ptr<Object>> Prepare(
        const std::vector<std::shared_ptr<Object>>&) override;
};

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
template <typename T, typename... Args>
std::shared_ptr<T> MakeNode(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}