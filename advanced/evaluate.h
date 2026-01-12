#pragma once

#include <object.h>
#include <scope.h>

ObjectPtr Evaluate(const ObjectPtr& program_ast, const std::shared_ptr<ScopesCollection>& scopes);
std::string Serialize(const ObjectPtr& root);