#include "evaluate.h"
#include "funcs.h"
#include "representation.h"

std::shared_ptr<Object> Evaluate(const std::shared_ptr<Object>& program_ast) {
    if (program_ast == nullptr) {
        return nullptr;
    }
    if (auto symbol = As<Symbol>(program_ast); symbol != nullptr) {
        if (symbol->GetName() == "#t" || symbol->GetName() == "#f") {
            return symbol;
        }
        return MakeFunction(symbol);
    }
    if (auto number = As<Number>(program_ast); number != nullptr) {
        return number;
    }
    auto root_cell = As<Cell>(program_ast);
    if (root_cell == nullptr) {
        throw RuntimeError("expected Cell");
    }
    auto function_obj = Evaluate(root_cell->GetFirst());
    auto arguments = Flatten(root_cell->GetSecond());

    if (arguments.back() != nullptr) {
        throw RuntimeError("improper list in function call");
    }
    arguments.pop_back();
    auto function = As<IFunction>(function_obj);
    if (function == nullptr) {
        throw RuntimeError("not a function");
    }
    auto result = function->Call(arguments);
    return ToAst(result);
}

std::string Serialize(const std::shared_ptr<Object>& root) {
    if (auto number = As<Number>(root); number != nullptr) {
        return std::to_string(number->GetValue());
    }
    if (auto symbol = As<Symbol>(root); symbol != nullptr) {
        return symbol->GetName();
    }
    auto flattened = Flatten(root);
    bool put_dot = flattened.back() != nullptr;
    if (flattened.back() == nullptr) {
        flattened.pop_back();
    }
    std::string answer = "(";
    for (size_t i = 0; i < flattened.size(); ++i) {
        answer += Serialize(flattened[i]);
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