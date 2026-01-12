#pragma once

#include <object.h>
#include <optional>
#include "error.h"

void AssertArgsCountEqual(const std::vector<std::shared_ptr<Object>>& args, size_t n);
void AssertArgsCountAtLeast(const std::vector<std::shared_ptr<Object>>& args, size_t n);

struct Maxer {
    IntType operator()(IntType first, IntType second);
};

struct Miner {
    IntType operator()(IntType first, IntType second);
};

struct Abser {
    IntType operator()(IntType number);
};

std::shared_ptr<IFunction> MakeFunction(const std::shared_ptr<Symbol>& symbol);

template <typename Type>
class IsType : public EvaluatingArgumentFunction {
public:
    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;
};

template <typename Type>
std::vector<std::shared_ptr<Object>> IsType<Type>::DoCall(
    const std::vector<std::shared_ptr<Object>>& args) {
    AssertArgsCountEqual(args, 1);

    return {MakeNode<Symbol>(ToBoolSymbol(Is<Type>(args.front())))};
}

class IsBoolean : public EvaluatingArgumentFunction {
public:
    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;
};

template <typename Comparator>
class Comparison : public EvaluatingArgumentFunction {
public:
    Comparison(Comparator comparator) : comparator_(std::move(comparator)) {
    }

    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;

private:
    Comparator comparator_;
};

const Number& GetNumber(const std::shared_ptr<Object>& object);

template <typename Comparator>
std::vector<std::shared_ptr<Object>> Comparison<Comparator>::DoCall(
    const std::vector<std::shared_ptr<Object>>& args) {
    bool answer = true;

    for (size_t i = 0; i + 1 < args.size(); ++i) {
        const auto& left = GetNumber(args[i]);
        const auto& right = GetNumber(args[i + 1]);

        if (!comparator_(left.GetValue(), right.GetValue())) {
            answer = false;
            break;
        }
    }

    return {MakeNode<Symbol>(ToBoolSymbol(answer))};
}

template <typename Op>
class BinaryApplier : public EvaluatingArgumentFunction {
public:
    BinaryApplier(Op op) : op_(std::move(op)) {
    }
    BinaryApplier(Op op, std::optional<Number> default_value)
        : op_(std::move(op)), default_value_(std::move(default_value)) {
    }

    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;

private:
    Op op_;
    std::optional<Number> default_value_ = std::nullopt;
};

template <typename Op>
std::vector<std::shared_ptr<Object>> BinaryApplier<Op>::DoCall(
    const std::vector<std::shared_ptr<Object>>& args) {
    if (args.empty()) {
        if (!default_value_.has_value()) {
            throw RuntimeError("expected value");
        }

        return {MakeNode<Number>(default_value_.value())};
    }

    auto answer = GetNumber(args.front());

    for (size_t i = 1; i < args.size(); ++i) {
        auto right = GetNumber(args[i]);
        answer = Number(op_(answer.GetValue(), right.GetValue()));
    }

    return {MakeNode<Number>(answer)};
}

template <typename Op>
class UnaryApplier : public EvaluatingArgumentFunction {
public:
    UnaryApplier(Op op) : op_(std::move(op)) {
    }

    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;

private:
    Op op_;
};

template <typename Op>
std::vector<std::shared_ptr<Object>> UnaryApplier<Op>::DoCall(
    const std::vector<std::shared_ptr<Object>>& args) {
    AssertArgsCountEqual(args, 1);

    auto answer = GetNumber(args.front());

    answer = op_(answer.GetValue());

    return {MakeNode<Number>(answer)};
}

class QuoteFunction : public UnevaluatingArgumentFunction {
public:
    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;
};

Symbol Alternate(const Symbol& symbol);

class Not : public EvaluatingArgumentFunction {
public:
    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;
};

class BoolExpressionEvaluator : public UnevaluatingArgumentFunction {
public:
    BoolExpressionEvaluator(bool expected);

    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;

private:
    bool expected_;
};

class IsPair : public EvaluatingArgumentFunction {
public:
    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;
};

class IsNull : public EvaluatingArgumentFunction {
public:
    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;
};

class IsList : public EvaluatingArgumentFunction {
public:
    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;
};

class Cons : public EvaluatingArgumentFunction {
public:
    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;
};

class Car : public EvaluatingArgumentFunction {
public:
    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;
};

class Cdr : public EvaluatingArgumentFunction {
public:
    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;
};

class List : public EvaluatingArgumentFunction {
public:
    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;
};

class ListRef : public EvaluatingArgumentFunction {
public:
    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;
};

class ListTail : public EvaluatingArgumentFunction {
public:
    std::vector<std::shared_ptr<Object>> DoCall(
        const std::vector<std::shared_ptr<Object>>& args) override;
};