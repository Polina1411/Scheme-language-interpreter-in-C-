#pragma once

#include <object.h>
#include <vector>
#include <memory>
std::vector<ObjectPtr> Flatten(const ObjectPtr& root);
ObjectPtr ToAst(const std::vector<ObjectPtr>& nodes);