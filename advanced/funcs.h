#pragma once

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <optional>
#include <object.h>
#include "representation.h"

template <typename... Args>
std::string FormatString(const std::string& format, Args&&... args) {
    std::ostringstream oss;
    size_t count = 0;

    auto write_args = [&oss, &count](auto&& arg) {
        if (count++ > 0) {
            oss << " ";
        }
        oss << std::forward<decltype(arg)>(arg);
    };

    oss << format;
    (write_args(std::forward<Args>(args)), ...);

    return oss.str();
}

template <typename Exception = InvalidArgsCount>
void AssertArgsCountEqual(const std::vector<ObjectPtr>& args, size_t n) {
    if (args.size() != n) {
        throw Exception(FormatString("expected", n, "got", args.size()));
    }
}

template <typename Exception = InvalidArgsCount>
void AssertArgsCountAtLeast(const std::vector<ObjectPtr>& args, size_t n) {
    if (args.size() < n) {
        throw Exception(FormatString("expected at least", n, "got", args.size()));
    }
}

template <typename Exception = InvalidArgsCount>
void AssertArgsCountBetween(const std::vector<ObjectPtr>& args, size_t start, size_t end) {
    if (!(args.size() >= start && args.size() <= end)) {
        throw Exception(FormatString("expected arguments amount in range from", start, "to", end,
                                     "got", args.size()));
    }
}

struct Maxer {
    IntType operator()(IntType first, IntType second);
};

struct Miner {
    IntType operator()(IntType first, IntType second);
};

struct Abser {
    IntType operator()(IntType number);
};

template <typename Type>
class IsType : public EvaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

template <typename Type>
std::vector<ObjectPtr> IsType<Type>::DoCall(const std::vector<ObjectPtr>& args,
                                            const std::shared_ptr<ScopesCollection>&) {
    AssertArgsCountEqual(args, 1);
    return {MakeNode<Boolean>(Is<Type>(args.front()))};
}

class IsBoolean : public EvaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

template <typename Comparator>
class Comparison : public EvaluatingArgumentFunction {
public:
    Comparison(Comparator comparator) : comparator_(std::move(comparator)) {
    }

    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);

private:
    Comparator comparator_;
};

const Number& GetNumber(const ObjectPtr& object);

template <typename Comparator>
std::vector<ObjectPtr> Comparison<Comparator>::DoCall(const std::vector<ObjectPtr>& args,
                                                      const std::shared_ptr<ScopesCollection>&) {
    bool answer = true;

    for (size_t i = 0; i + 1 < args.size(); ++i) {
        const auto& left = GetNumber(args[i]);
        const auto& right = GetNumber(args[i + 1]);

        if (!comparator_(left.GetValue(), right.GetValue())) {
            answer = false;
            break;
        }
    }

    return {MakeNode<Boolean>(answer)};
}

template <typename Op>
class BinaryApplier : public EvaluatingArgumentFunction {
public:
    BinaryApplier(Op op) : op_(std::move(op)) {
    }
    BinaryApplier(Op op, std::optional<Number> default_value)
        : op_(std::move(op)), default_value_(std::move(default_value)) {
    }

    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);

private:
    Op op_;
    std::optional<Number> default_value_ = std::nullopt;
};

template <typename Op>
std::vector<ObjectPtr> BinaryApplier<Op>::DoCall(const std::vector<ObjectPtr>& args,
                                                 const std::shared_ptr<ScopesCollection>&) {
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

    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);

private:
    Op op_;
};

template <typename Op>
std::vector<ObjectPtr> UnaryApplier<Op>::DoCall(const std::vector<ObjectPtr>& args,
                                                const std::shared_ptr<ScopesCollection>&) {
    AssertArgsCountEqual(args, 1);
    auto answer = GetNumber(args.front());
    answer = Number(op_(answer.GetValue()));
    return {MakeNode<Number>(answer)};
}

class QuoteFunction : public UnevaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class Not : public EvaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class BoolExpressionEvaluator : public UnevaluatingArgumentFunction {
public:
    BoolExpressionEvaluator(bool expected);

    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);

private:
    bool expected_;
};

class IsPair : public EvaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class IsNull : public EvaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class IsList : public EvaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class Cons : public EvaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class Car : public EvaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class Cdr : public EvaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class List : public EvaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class ListRef : public EvaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class ListTail : public EvaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class If : public UnevaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class Define : public UnevaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class Set : public UnevaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class SetCar : public EvaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class SetCdr : public EvaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

class Lambda : public UnevaluatingArgumentFunction {
public:
    Lambda(std::vector<ObjectPtr> args, const std::shared_ptr<ScopesCollection>& scopes)
        : captured_scopes_(scopes) {
        AssertArgsCountAtLeast<SyntaxError>(args, 2);

        auto flattened_args_list = Flatten(args.front());
        flattened_args_list.pop_back();
        std::vector<std::shared_ptr<Symbol>> arguments_list;
        arguments_list.reserve(flattened_args_list.size());
        for (const auto& ptr : flattened_args_list) {
            auto symbol = As<Symbol>(ptr);
            if (symbol == nullptr) {
                throw SyntaxError("argument should be a symbol");
            }

            arguments_list.push_back(std::move(symbol));
        }
        arguments_list_ = std::move(arguments_list);

        body_ = {args.begin() + 1, args.end()};
    }

    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);

private:
    std::shared_ptr<ScopesCollection> captured_scopes_;
    std::vector<std::shared_ptr<Symbol>> arguments_list_;
    std::vector<ObjectPtr> body_;
};

class LambdaMaker : public UnevaluatingArgumentFunction {
public:
    std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes);
};

std::shared_ptr<Scope> CreateBuiltinsScope();
std::shared_ptr<Scope> GetBuiltinsScope();