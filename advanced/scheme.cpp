#include "scheme.h"
#include "scope.h"
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <tokenizer.h>
#include <parser.h>
#include <funcs.h>
#include <evaluate.h>

Interpreter::Interpreter()
    : scope_(std::make_shared<Scope>(std::unordered_map<std::string, ObjectPtr>{},
                                     GetBuiltinsScope())) {
}
std::string Interpreter::Run(const std::string& program) {
    std::stringstream string_stream{program};
    Tokenizer tokenizer{&string_stream};

    auto program_ast = Read(&tokenizer);
    auto scopes = std::shared_ptr<ScopesCollection>(
        new ScopesCollection(std::vector<std::shared_ptr<Scope>>{scope_}));
    auto evaluation_result_ast = Evaluate(program_ast, scopes);

    return Serialize(evaluation_result_ast);
}