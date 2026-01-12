#include "object.h"
#include "evaluate.h"
Symbol ToBoolSymbol(bool arg) {
    if (arg) {
        return kSTrue;
    } else {
        return kSFalse;
    }
}
bool ToBool(const Symbol& symbol) {
    if (symbol.GetName() == kSFalse.GetName()) {
        return false;
    }
    return true;
}
bool ToBool(const std::shared_ptr<Object>& object) {
    if (auto elem = As<Symbol>(object); elem != nullptr) {
        return ToBool(*elem);
    }
    return true;
}
std::vector<std::shared_ptr<Object>> IFunction::Call(
    const std::vector<std::shared_ptr<Object>>& args) {
    auto new_args = Prepare(args);
    return DoCall(new_args);
}
std::vector<std::shared_ptr<Object>> EvaluatingArgumentFunction::Prepare(
    const std::vector<std::shared_ptr<Object>>& args) {
    auto res = args;
    for (auto i = 0; i < res.size(); ++i) {
        res[i] = Evaluate(res[i]);
    }
    return res;
}
std::vector<std::shared_ptr<Object>> UnevaluatingArgumentFunction::Prepare(
    const std::vector<std::shared_ptr<Object>>& args) {
    return args;
}
