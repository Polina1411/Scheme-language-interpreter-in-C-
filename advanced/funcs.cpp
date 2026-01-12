#include "funcs.h"

#include "evaluate.h"
#include "representation.h"

IntType Maxer::operator()(IntType first, IntType second) {
    return std::max(first, second);
}

IntType Miner::operator()(IntType first, IntType second) {
    return std::min(first, second);
}

IntType Abser::operator()(IntType number) {
    return std::abs(number);
}

const Number& GetNumber(const ObjectPtr& object) {
    auto number = As<Number>(object);
    if (number == nullptr) {
        throw RuntimeError("expected number");
    }

    return *number;
}

std::vector<ObjectPtr> IsBoolean::DoCall(const std::vector<ObjectPtr>& args,
                                         const std::shared_ptr<ScopesCollection>&) {
    AssertArgsCountEqual(args, 1);

    return {MakeNode<Boolean>(Is<Boolean>(args.front()))};
}

std::vector<ObjectPtr> QuoteFunction::DoCall(const std::vector<ObjectPtr>& args,
                                             const std::shared_ptr<ScopesCollection>&) {
    AssertArgsCountEqual(args, 1);

    return args;
}

std::vector<ObjectPtr> Not::DoCall(const std::vector<ObjectPtr>& args,
                                   const std::shared_ptr<ScopesCollection>&) {
    AssertArgsCountEqual(args, 1);

    const auto& arg = As<Boolean>(args.front());
    bool answer = arg != nullptr ? (!arg->GetValue()) : false;

    return {MakeNode<Boolean>(answer)};
}

BoolExpressionEvaluator::BoolExpressionEvaluator(bool expected) : expected_(expected) {
}

std::vector<ObjectPtr> BoolExpressionEvaluator::DoCall(
    const std::vector<ObjectPtr>& args, const std::shared_ptr<ScopesCollection>& scopes) {
    if (args.empty()) {
        return {MakeNode<Boolean>(!expected_)};
    }

    ObjectPtr evaluated_arg;
    for (size_t i = 0; i < args.size(); ++i) {
        evaluated_arg = ::Evaluate(args[i], scopes);
        bool res = ToBool(evaluated_arg);
        if (res == expected_) {
            break;
        }
    }

    return {evaluated_arg};
}

std::vector<ObjectPtr> IsPair::DoCall(const std::vector<ObjectPtr>& args,
                                      const std::shared_ptr<ScopesCollection>&) {
    AssertArgsCountEqual(args, 1);

    auto flattened = Flatten(args.front());
    if (!flattened.empty() && flattened.back() == nullptr) {
        flattened.pop_back();
    }
    return {MakeNode<Boolean>(flattened.size() == 2)};
}

std::vector<ObjectPtr> IsNull::DoCall(const std::vector<ObjectPtr>& args,
                                      const std::shared_ptr<ScopesCollection>&) {
    AssertArgsCountEqual(args, 1);

    return {MakeNode<Boolean>(args.front() == nullptr)};
}

std::vector<ObjectPtr> IsList::DoCall(const std::vector<ObjectPtr>& args,
                                      const std::shared_ptr<ScopesCollection>&) {
    AssertArgsCountEqual(args, 1);

    auto flattened = Flatten(args.front());
    return {MakeNode<Boolean>(flattened.empty() || flattened.back() == nullptr)};
}

std::vector<ObjectPtr> Cons::DoCall(const std::vector<ObjectPtr>& args,
                                    const std::shared_ptr<ScopesCollection>&) {
    AssertArgsCountEqual(args, 2);

    return args;
}

void AssertIsNotEmptyList(const std::vector<ObjectPtr>& list) {
    if (list.front() == nullptr) {
        throw RuntimeError("expected non-empty list");
    }
}

std::vector<ObjectPtr> Car::DoCall(const std::vector<ObjectPtr>& args,
                                   const std::shared_ptr<ScopesCollection>&) {
    AssertArgsCountEqual(args, 1);

    auto cell = As<Cell>(args.front());
    if (cell == nullptr) {
        throw RuntimeError("not a list");
    }

    return {cell->GetFirst()};
}

std::vector<ObjectPtr> Cdr::DoCall(const std::vector<ObjectPtr>& args,
                                   const std::shared_ptr<ScopesCollection>&) {
    AssertArgsCountEqual(args, 1);

    auto cell = As<Cell>(args.front());
    if (cell == nullptr) {
        throw RuntimeError("not a list");
    }

    return {cell->GetSecond()};
}

std::vector<ObjectPtr> List::DoCall(const std::vector<ObjectPtr>& args,
                                    const std::shared_ptr<ScopesCollection>&) {
    auto answer = args;
    answer.push_back(nullptr);
    return answer;
}

bool IsInBounds(IntType index, size_t size) {
    return index >= 0 && static_cast<size_t>(index) < size;
}

std::vector<ObjectPtr> ListRef::DoCall(const std::vector<ObjectPtr>& args,
                                       const std::shared_ptr<ScopesCollection>&) {
    AssertArgsCountEqual(args, 2);

    auto flattened = Flatten(args.front());
    auto index_num = GetNumber(args.back());
    IntType index = index_num.GetValue();

    if (!IsInBounds(index, flattened.size() - 1)) {
        throw RuntimeError("index out of bounds");
    }

    return {flattened[index]};
}

std::vector<ObjectPtr> ListTail::DoCall(const std::vector<ObjectPtr>& args,
                                        const std::shared_ptr<ScopesCollection>&) {
    AssertArgsCountEqual(args, 2);

    auto flattened = Flatten(args.front());
    auto index_num = GetNumber(args.back());
    IntType index = index_num.GetValue();

    if (!IsInBounds(index, flattened.size())) {
        throw RuntimeError("index out of bounds");
    }

    return {flattened.begin() + index, flattened.end()};
}

std::vector<ObjectPtr> If::DoCall(const std::vector<ObjectPtr>& args,
                                  const std::shared_ptr<ScopesCollection>& scopes) {
    AssertArgsCountBetween<SyntaxError>(args, 2, 3);

    bool condition = ToBool(::Evaluate(args.front(), scopes));
    if (condition) {
        return Flatten(::Evaluate(args[1], scopes));
    }

    if (!condition && args.size() > 2) {
        return Flatten(::Evaluate(args[2], scopes));
    }

    return {nullptr};
}

std::shared_ptr<Symbol> GetSymbol(const ObjectPtr& object) {
    auto symbol = As<Symbol>(object);
    if (symbol == nullptr) {
        throw RuntimeError("not a symbol");
    }

    return symbol;
}

struct ScopeSetArgs {
    std::string key;
    ObjectPtr value;
    bool in_last_scope;
};

ScopeSetArgs MakeScopeSetArgs(const std::vector<ObjectPtr>& args,
                              const std::shared_ptr<ScopesCollection>& scopes) {
    AssertArgsCountAtLeast<SyntaxError>(args, 1);

    if (auto cell = As<Cell>(args.front()); cell != nullptr) {
        AssertArgsCountAtLeast<SyntaxError>(args, 2);
        auto func_name = As<Symbol>(cell->GetFirst());
        if (func_name == nullptr) {
            throw SyntaxError("not a fucntion name");
        }

        auto arguments_list = cell->GetSecond();
        std::vector<ObjectPtr> lambda_args = {args.begin() + 1, args.end()};
        lambda_args.insert(lambda_args.begin(), arguments_list);

        auto lambda = std::make_shared<Lambda>(lambda_args, scopes);
        return {.key = func_name->GetName(), .value = lambda, .in_last_scope = true};
    }

    AssertArgsCountEqual<SyntaxError>(args, 2);
    auto symbol = GetSymbol(args.front());

    return {
        .key = symbol->GetName(),
        .value = ::Evaluate(args[1], scopes),
        .in_last_scope = true,
    };
}

std::vector<ObjectPtr> Define::DoCall(const std::vector<ObjectPtr>& args,
                                      const std::shared_ptr<ScopesCollection>& scopes) {
    auto scope_set_args = MakeScopeSetArgs(args, scopes);

    scopes->Set(scope_set_args.key, scope_set_args.value, scope_set_args.in_last_scope);
    return {nullptr};
}

std::vector<ObjectPtr> Set::DoCall(const std::vector<ObjectPtr>& args,
                                   const std::shared_ptr<ScopesCollection>& scopes) {
    AssertArgsCountEqual<SyntaxError>(args, 2);

    auto symbol = GetSymbol(args.front());

    if (scopes->Get(symbol->GetName()) == nullptr) {
        throw NameError("no such object");
    }

    bool in_first_scope = false;
    scopes->Set(symbol->GetName(), ::Evaluate(args[1], scopes), in_first_scope);

    return {nullptr};
}

std::vector<ObjectPtr> SetCar::DoCall(const std::vector<ObjectPtr>& args,
                                      const std::shared_ptr<ScopesCollection>&) {
    AssertArgsCountEqual(args, 2);

    auto cell = As<Cell>(args.front());
    if (cell == nullptr) {
        throw RuntimeError("not a pair");
    }

    cell->SetFirst(args[1]);
    return {nullptr};
}

std::vector<ObjectPtr> SetCdr::DoCall(const std::vector<ObjectPtr>& args,
                                      const std::shared_ptr<ScopesCollection>&) {
    AssertArgsCountEqual(args, 2);

    auto cell = As<Cell>(args.front());
    if (cell == nullptr) {
        throw RuntimeError("not a pair");
    }

    cell->SetSecond(args[1]);
    return {nullptr};
}

std::vector<ObjectPtr> Lambda::DoCall(const std::vector<ObjectPtr>& args,
                                      const std::shared_ptr<ScopesCollection>& scopes) {
    AssertArgsCountEqual(args, arguments_list_.size());

    std::unordered_map<std::string, std::shared_ptr<Object>> args_symbols;
    for (size_t i = 0; i < arguments_list_.size(); ++i) {
        args_symbols[arguments_list_[i]->GetName()] = ::Evaluate(args[i], scopes);
    }
    auto args_scope = std::make_shared<Scope>(args_symbols, nullptr);
    auto lambda_scopes =
        std::make_shared<ScopesCollection>(std::vector<std::shared_ptr<Scope>>{args_scope});
    lambda_scopes->AddScopes(captured_scopes_);
    lambda_scopes->AddScopes(scopes);

    for (size_t i = 0; i < body_.size(); ++i) {
        auto res = ::Evaluate(body_[i], lambda_scopes);
        if (i + 1 == body_.size()) {
            return {res};
        }
    }

    return {nullptr};
}

std::vector<ObjectPtr> LambdaMaker::DoCall(const std::vector<ObjectPtr>& args,
                                           const std::shared_ptr<ScopesCollection>& scopes) {
    return {std::make_shared<Lambda>(args, scopes)};
}

std::shared_ptr<Scope> CreateBuiltinsScope() {
    return std::make_shared<Scope>(
        std::unordered_map<std::string, std::shared_ptr<Object>>{
            {"number?", MakeNode<IsType<Number>>()},
            {"<", MakeNode<Comparison<std::less<IntType>>>(std::less<IntType>{})},
            {"=", MakeNode<Comparison<std::equal_to<IntType>>>(std::equal_to<IntType>{})},
            {">", MakeNode<Comparison<std::greater<IntType>>>(std::greater<IntType>{})},
            {"<=", MakeNode<Comparison<std::less_equal<IntType>>>(std::less_equal<IntType>{})},
            {">=",
             MakeNode<Comparison<std::greater_equal<IntType>>>(std::greater_equal<IntType>{})},
            {"+", MakeNode<BinaryApplier<std::plus<IntType>>>(std::plus<IntType>{}, Number(0))},
            {"-", MakeNode<BinaryApplier<std::minus<IntType>>>(std::minus<IntType>{})},
            {"/", MakeNode<BinaryApplier<std::divides<IntType>>>(std::divides<IntType>{})},
            {"*", MakeNode<BinaryApplier<std::multiplies<IntType>>>(std::multiplies<IntType>{},
                                                                    Number(1))},
            {"max", MakeNode<BinaryApplier<Maxer>>(Maxer{})},
            {"min", MakeNode<BinaryApplier<Miner>>(Miner{})},
            {"abs", MakeNode<UnaryApplier<Abser>>(Abser{})},

            {"boolean?", MakeNode<IsBoolean>()},
            {"quote", MakeNode<QuoteFunction>()},
            {"not", MakeNode<Not>()},
            {"and", MakeNode<BoolExpressionEvaluator>(false)},
            {"or", MakeNode<BoolExpressionEvaluator>(true)},

            {"pair?", MakeNode<IsPair>()},
            {"null?", MakeNode<IsNull>()},
            {"list?", MakeNode<IsList>()},

            {"cons", MakeNode<Cons>()},
            {"car", MakeNode<Car>()},
            {"cdr", MakeNode<Cdr>()},
            {"list", MakeNode<List>()},
            {"list-ref", MakeNode<ListRef>()},
            {"list-tail", MakeNode<ListTail>()},

            {"if", MakeNode<If>()},
            {"define", MakeNode<Define>()},
            {"symbol?", MakeNode<IsType<Symbol>>()},
            {"set!", MakeNode<Set>()},
            {"set-car!", MakeNode<SetCar>()},
            {"set-cdr!", MakeNode<SetCdr>()},

            {"lambda", MakeNode<LambdaMaker>()},
        },
        nullptr);
}
std::shared_ptr<Scope> GetBuiltinsScope() {
    static std::shared_ptr<Scope> kBuiltins = CreateBuiltinsScope();
    return kBuiltins;
}