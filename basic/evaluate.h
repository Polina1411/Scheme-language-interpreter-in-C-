#pragma once

#include "object.h"
#include "error.h"
std::shared_ptr<Object> Evaluate(const std::shared_ptr<Object>& program_ast);
std::string Serialize(const std::shared_ptr<Object>& root);
