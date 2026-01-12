#pragma once

#include <memory>

#include "object.h"
#include <tokenizer.h>
#include <utility>
#include <error.h>
#include <tokenizer.cpp>
std::shared_ptr<Object> Read(Tokenizer* tokenizer);
std::shared_ptr<Object> ReadList(Tokenizer* tokenizer);
std::shared_ptr<Symbol> ReadQuote();