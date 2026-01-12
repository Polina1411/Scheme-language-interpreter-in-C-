#include "evaluate.h"
#include <funcs.h>
#include <representation.h>

ObjectPtr Evaluate(const ObjectPtr& program_ast, const std::shared_ptr<ScopesCollection>& scopes) {
    if (program_ast == nullptr) {
        return nullptr;
    }

    return program_ast->Evaluate(scopes);
}

std::string Serialize(const ObjectPtr& root) {
    if (root == nullptr) {
        return "()";
    }

    return root->Serialize();
}