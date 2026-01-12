#pragma once

#include <variant>
#include <optional>
#include <istream>
#include <cctype>
#include <array>
#include <algorithm>
#include "error.h"

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const {
        return name == other.name;
    }
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const {
        return true;
    }
};

struct DotToken {
    bool operator==(const DotToken&) const {
        return true;
    }
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    bool operator==(const ConstantToken& other) const {
        return value == other.value;
    }
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken>;

class Tokenizer {
private:
    std::istream* in_;
    Token temp_token_;
    bool reach_end_ = false;
    std::string ReadWholeNumber() {
        std::string str;
        while (std::isdigit(in_->peek())) {
            auto addition = in_->get();
            str += addition;
        }
        return str;
    }  // НЕ ЗАБЫТЬ РАПИСАТЬ ВОЗМОЖНЫЕ СТМВОЛЫ АЗ И ТД!!!
    std::array<char, 2> signs_ = {'+', '-'};
    std::array<char, 6> signs_begin_ = {'<', '=', '>', '*', '/', '#'};
    std::array<char, 4> sings_contain_ = {'/', '?', '!', '-'};
    bool AllowedBegin(char c) {
        return std::find(signs_begin_.begin(), signs_begin_.end(), c) != signs_begin_.end() ||
               std::isalpha(c);
    }
    bool AllowedTail(char c) {
        return AllowedBegin(c) ||
               std::find(sings_contain_.begin(), sings_contain_.end(), c) != sings_contain_.end() ||
               std::isdigit(c);
    }
    std::string SubmitTail() {
        std::string str;
        while (AllowedTail(in_->peek())) {
            str += in_->get();
        }
        return str;
    }

public:
    Tokenizer(std::istream* in) : in_(in) {
        Next();
    }

    bool IsEnd() {
        return reach_end_;
    }

    void Next() {
        char c;
        while (std::isspace(in_->peek())) {
            in_->get();
        }
        if (in_->eof() || in_->peek() == EOF) {
            reach_end_ = true;
            return;
        }
        c = in_->get();
        std::string cur_token = {c};
        if (c == '.') {
            temp_token_ = DotToken{};
            return;
        }
        if (c == '\'') {
            temp_token_ = QuoteToken{};
            return;
        }
        if (c == '(') {
            temp_token_ = BracketToken::OPEN;
            return;
        }
        if (c == ')') {
            temp_token_ = BracketToken::CLOSE;
            return;
        }
        if (std::isdigit(c)) {
            cur_token += ReadWholeNumber();
            temp_token_ = ConstantToken{std::stoi(cur_token)};
            return;
        }
        if (std::find(signs_.begin(), signs_.end(), c) != signs_.end()) {
            std::string read = ReadWholeNumber();
            if (read.empty()) {
                temp_token_ = SymbolToken{cur_token};
                return;
            }
            cur_token += read;
            temp_token_ = ConstantToken{std::stoi(cur_token)};
            return;
        }
        if (AllowedBegin(c)) {
            cur_token += SubmitTail();
            temp_token_ = SymbolToken{cur_token};
            return;
        }
        throw SyntaxError(std::string("Unknown token: ") + c);
    }

    Token GetToken() {
        return temp_token_;
    }
};