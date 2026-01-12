#pragma once

#include "object.h"

std::vector<std::shared_ptr<Object>> Flatten(const std::shared_ptr<Object>& root);
std::shared_ptr<Object> ToAst(const std::vector<std::shared_ptr<Object>>& nodes);