#include "scheme.h"

#include <algorithm>
#include <sstream>
#include <unordered_map>

#include "tokenizer.h"
#include "parser.h"
#include "funcs.h"
#include "evaluate.h"

std::string Interpreter::Run(const std::string& program) {
    std::stringstream string_stream{program};
    Tokenizer tokenizer{&string_stream};
    auto program_ast = Read(&tokenizer);
    auto evaluation_result_ast = Evaluate(program_ast);
    if (evaluation_result_ast != nullptr &&
        (!Is<Number>(evaluation_result_ast) && !Is<Symbol>(evaluation_result_ast) &&
         !Is<Cell>(evaluation_result_ast))) {
        throw RuntimeError("not a value");
    }

    return Serialize(evaluation_result_ast);
}