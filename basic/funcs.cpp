#include "funcs.h"
#include "evaluate.h"
#include "representation.h"

std::shared_ptr<IFunction> MakeFunction(const std::shared_ptr<Symbol>& symbol) {
    const auto name = symbol->GetName();

    if (name == "boolean?") {
        return MakeNode<IsBoolean>();
    }
    if (name == "not") {
        return MakeNode<Not>();
    }
    if (name == "and") {
        return MakeNode<BoolExpressionEvaluator>(false);
    }
    if (name == "or") {
        return MakeNode<BoolExpressionEvaluator>(true);
    }

    if (name == "number?") {
        return MakeNode<IsType<Number>>();
    }
    if (name == "pair?") {
        return MakeNode<IsPair>();
    }
    if (name == "null?") {
        return MakeNode<IsNull>();
    }
    if (name == "list?") {
        return MakeNode<IsList>();
    }

    if (name == "+") {
        return MakeNode<BinaryApplier<std::plus<IntType>>>(std::plus<IntType>{}, Number(0));
    }
    if (name == "-") {
        return MakeNode<BinaryApplier<std::minus<IntType>>>(std::minus<IntType>{});
    }
    if (name == "*") {
        return MakeNode<BinaryApplier<std::multiplies<IntType>>>(std::multiplies<IntType>{},
                                                                 Number(1));
    }
    if (name == "/") {
        return MakeNode<BinaryApplier<std::divides<IntType>>>(std::divides<IntType>{});
    }
    if (name == "max") {
        return MakeNode<BinaryApplier<Maxer>>(Maxer{});
    }
    if (name == "min") {
        return MakeNode<BinaryApplier<Miner>>(Miner{});
    }
    if (name == "abs") {
        return MakeNode<UnaryApplier<Abser>>(Abser{});
    }

    if (name == "<") {
        return MakeNode<Comparison<std::less<IntType>>>(std::less<IntType>{});
    }
    if (name == ">") {
        return MakeNode<Comparison<std::greater<IntType>>>(std::greater<IntType>{});
    }
    if (name == "=") {
        return MakeNode<Comparison<std::equal_to<IntType>>>(std::equal_to<IntType>{});
    }
    if (name == "<=") {
        return MakeNode<Comparison<std::less_equal<IntType>>>(std::less_equal<IntType>{});
    }
    if (name == ">=") {
        return MakeNode<Comparison<std::greater_equal<IntType>>>(std::greater_equal<IntType>{});
    }

    if (name == "cons") {
        return MakeNode<Cons>();
    }
    if (name == "car") {
        return MakeNode<Car>();
    }
    if (name == "cdr") {
        return MakeNode<Cdr>();
    }
    if (name == "list") {
        return MakeNode<List>();
    }
    if (name == "list-ref") {
        return MakeNode<ListRef>();
    }
    if (name == "list-tail") {
        return MakeNode<ListTail>();
    }

    if (name == "quote") {
        return MakeNode<QuoteFunction>();
    }

    throw RuntimeError("unknown function");
}

IntType Maxer::operator()(IntType first, IntType second) {
    return first > second ? first : second;
}
IntType Miner::operator()(IntType first, IntType second) {
    return first < second ? first : second;
}
IntType Abser::operator()(IntType number) {
    return std::abs(number);
}
void AssertArgsCountEqual(const std::vector<std::shared_ptr<Object>>& args, size_t n) {
    if (args.size() != n) {
        throw InvalidArgsCount("expected " + std::to_string(n) + ", got " +
                               std::to_string(args.size()));
    }
}
void AssertArgsCountAtLeast(const std::vector<std::shared_ptr<Object>>& args, size_t n) {
    if (args.size() < n) {
        throw InvalidArgsCount("expected at least " + std::to_string(n) + ", got " +
                               std::to_string(args.size()));
    }
}
const Number& GetNumber(const std::shared_ptr<Object>& object) {
    auto number = As<Number>(object);
    if (number == nullptr) {
        throw RuntimeError("no expression");
    }
    return *number;
}
std::vector<std::shared_ptr<Object>> IsBoolean::DoCall(
    const std::vector<std::shared_ptr<Object>>& args) {
    AssertArgsCountEqual(args, 1);
    bool answer = false;
    auto symbol = As<Symbol>(args.front());
    if (symbol != nullptr) {
        std::string name = symbol->GetName();
        for (const auto& bool_const : kBoolConsts) {
            if (bool_const.GetName() == name) {
                answer = true;
                break;
            }
        }
    }
    return {MakeNode<Symbol>(ToBoolSymbol(answer))};
}
std::vector<std::shared_ptr<Object>> QuoteFunction::DoCall(
    const std::vector<std::shared_ptr<Object>>& args) {
    AssertArgsCountEqual(args, 1);
    return args;
}
Symbol Alternate(const Symbol& symbol) {
    std::string name = symbol.GetName();
    if (name == "#t") {
        return ToBoolSymbol(false);
    }
    if (name == "#f") {
        return ToBoolSymbol(true);
    }
    return symbol;
}

std::vector<std::shared_ptr<Object>> Not::DoCall(const std::vector<std::shared_ptr<Object>>& args) {
    AssertArgsCountEqual(args, 1);

    auto arg = As<Symbol>(args.front());
    if (arg == nullptr) {
        return {MakeNode<Symbol>(kSFalse)};
    }

    return {MakeNode<Symbol>(Alternate(*arg))};
}

BoolExpressionEvaluator::BoolExpressionEvaluator(bool expected) : expected_(expected) {
}

std::vector<std::shared_ptr<Object>> BoolExpressionEvaluator::DoCall(
    const std::vector<std::shared_ptr<Object>>& args) {
    if (args.empty()) {
        return {MakeNode<Symbol>(ToBoolSymbol(!expected_))};
    }

    std::shared_ptr<Object> evaluated_arg;
    for (size_t i = 0; i < args.size(); ++i) {
        evaluated_arg = Evaluate(args[i]);
        bool res = ToBool(evaluated_arg);
        if (res == expected_) {
            break;
        }
    }

    return {evaluated_arg};
}
std::vector<std::shared_ptr<Object>> IsPair::DoCall(
    const std::vector<std::shared_ptr<Object>>& args) {
    AssertArgsCountEqual(args, 1);

    auto flattened = Flatten(args.front());
    if (!flattened.empty() && flattened.back() == nullptr) {
        flattened.pop_back();
    }
    return {MakeNode<Symbol>(ToBoolSymbol(flattened.size() == 2))};
}

std::vector<std::shared_ptr<Object>> IsNull::DoCall(
    const std::vector<std::shared_ptr<Object>>& args) {
    AssertArgsCountEqual(args, 1);
    return {MakeNode<Symbol>(ToBoolSymbol(args.front() == nullptr))};
}

std::vector<std::shared_ptr<Object>> IsList::DoCall(
    const std::vector<std::shared_ptr<Object>>& args) {
    AssertArgsCountEqual(args, 1);
    auto flattened = Flatten(args.front());
    return {MakeNode<Symbol>(ToBoolSymbol(flattened.empty() || flattened.back() == nullptr))};
}

std::vector<std::shared_ptr<Object>> Cons::DoCall(
    const std::vector<std::shared_ptr<Object>>& args) {
    AssertArgsCountEqual(args, 2);
    return args;
}

std::vector<std::shared_ptr<Object>> Car::DoCall(const std::vector<std::shared_ptr<Object>>& args) {
    AssertArgsCountEqual(args, 1);
    auto flattened = Flatten(args.front());
    if (flattened.front() == nullptr) {
        throw RuntimeError("expected non-empty list");
    }
    return {flattened.front()};
}

std::vector<std::shared_ptr<Object>> Cdr::DoCall(const std::vector<std::shared_ptr<Object>>& args) {
    AssertArgsCountEqual(args, 1);
    auto flattened = Flatten(args.front());
    if (flattened.front() == nullptr) {
        throw RuntimeError("expected non-empty list");
    }
    flattened.erase(flattened.begin());
    return flattened;
}

std::vector<std::shared_ptr<Object>> List::DoCall(
    const std::vector<std::shared_ptr<Object>>& args) {
    auto answer = args;
    answer.push_back(nullptr);
    return answer;
}

std::vector<std::shared_ptr<Object>> ListRef::DoCall(
    const std::vector<std::shared_ptr<Object>>& args) {
    AssertArgsCountEqual(args, 2);
    auto flattened = Flatten(args.front());
    auto index_num = As<Number>(args.back());
    if (index_num == nullptr) {
        throw RuntimeError("expected number as index");
    }
    IntType index = index_num->GetValue();
    size_t elements_count = flattened.size() - 1;
    if (index < 0 || static_cast<size_t>(index) >= elements_count) {
        throw RuntimeError("index out of bounds");
    }
    return {flattened[index]};
}

std::vector<std::shared_ptr<Object>> ListTail::DoCall(
    const std::vector<std::shared_ptr<Object>>& args) {
    AssertArgsCountEqual(args, 2);
    auto flattened = Flatten(args.front());
    auto index_num = As<Number>(args.back());
    if (index_num == nullptr) {
        throw RuntimeError("expected number as index");
    }
    IntType index = index_num->GetValue();
    size_t elems_count = flattened.size() - 1;
    if (static_cast<size_t>(index) > elems_count) {
        throw RuntimeError("index out of bounds");
    }
    return {flattened.begin() + index, flattened.end()};
}