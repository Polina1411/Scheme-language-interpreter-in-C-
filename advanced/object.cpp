#include "object.h"
#include <vector>
#include <memory>
#include <evaluate.h>
#include "representation.h"

std::vector<ObjectPtr> IFunction::Call(const std::vector<ObjectPtr>& args,
                                       const std::shared_ptr<ScopesCollection>& scopes) {
    auto prepared_args = Prepare(args, scopes);
    return DoCall(prepared_args, scopes);
}

std::vector<ObjectPtr> EvaluatingArgumentFunction::Prepare(
    const std::vector<ObjectPtr>& args, const std::shared_ptr<ScopesCollection>& scopes) {
    auto answer = args;
    for (size_t i = 0; i < answer.size(); ++i) {
        answer[i] = ::Evaluate(answer[i], scopes);
    }

    return answer;
}

std::vector<ObjectPtr> UnevaluatingArgumentFunction::Prepare(
    const std::vector<ObjectPtr>& args, const std::shared_ptr<ScopesCollection>&) {
    return args;
}

bool ToBool(const ObjectPtr& object) {
    if (auto symbol = As<Boolean>(object); symbol != nullptr) {
        return symbol->GetValue();
    }

    return true;
}

std::shared_ptr<Object> Object::Clone() {
    return shared_from_this();
}

std::string Number::Serialize() {
    return std::to_string(value_);
}

std::string Symbol::Serialize() {
    return name_;
}

std::string Boolean::Serialize() {
    return value_ ? "#t" : "#f";
}

std::string Cell::Serialize() {
    auto flattened = Flatten(Clone());
    bool put_dot = flattened.back() != nullptr;
    if (flattened.back() == nullptr) {
        flattened.pop_back();
    }

    std::string answer = "(";
    for (size_t i = 0; i < flattened.size(); ++i) {
        answer += ::Serialize(flattened[i]);
        if (i + 1 != flattened.size()) {
            answer += " ";
        }
        if (i + 2 == flattened.size() && put_dot) {
            answer += ". ";
        }
    }
    answer += ")";

    return answer;
}

std::string IFunction::Serialize() {
    throw RuntimeError("function unserializable");
}

ObjectPtr Number::Evaluate(const std::shared_ptr<ScopesCollection>&) {
    return Clone();
}

ObjectPtr Symbol::Evaluate(const std::shared_ptr<ScopesCollection>& scopes) {
    auto value = scopes->Get(name_);
    if (value == nullptr) {
        throw NameError("no such object");
    }
    return value;
}

ObjectPtr Boolean::Evaluate(const std::shared_ptr<ScopesCollection>&) {
    return Clone();
}

ObjectPtr Cell::Evaluate(const std::shared_ptr<ScopesCollection>& scope) {
    auto function_obj = ::Evaluate(GetFirst(), scope);
    auto arguments = Flatten(GetSecond());
    if (arguments.back() != nullptr) {
        throw RuntimeError("can't call function with inproper list");
    }
    arguments.pop_back();

    std::shared_ptr<IFunction> function = As<IFunction>(function_obj);
    if (function == nullptr) {
        throw RuntimeError("not a function");
    }

    auto result = function->Call(arguments, scope);
    return ToAst(result);
}

ObjectPtr IFunction::Evaluate(const std::shared_ptr<ScopesCollection>&) {
    throw RuntimeError("function unevaluatable");
}