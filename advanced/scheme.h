#pragma once

#include <string>
#include "scope.h"
#include <vector>
#include <memory>
#include "scope_fwd.h"

class Interpreter {
public:
    Interpreter();

    std::string Run(const std::string& program);

private:
    std::shared_ptr<Scope> scope_;
};