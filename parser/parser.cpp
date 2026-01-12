#include <parser.h>

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("expect not an empty expression");
    }
    Token token = tokenizer->GetToken();
    switch (token.index()) {
        case 0: {
            auto& elem = std::get<ConstantToken>(token);
            tokenizer->Next();
            return std::make_shared<Number>(elem.value);
        }
        case 1: {
            auto& bracket = std::get<BracketToken>(token);
            if (bracket == BracketToken::OPEN) {
                return ReadList(tokenizer);
            } else {
                throw SyntaxError("closing");
            }
        }
        case 2: {
            auto& elem = std::get<SymbolToken>(token);
            tokenizer->Next();
            return std::make_shared<Symbol>(std::move(elem.name));
        }
        case 3: {
            throw SyntaxError("quote");
        }
        case 4: {
            throw SyntaxError("dot");
        }
        default:
            throw SyntaxError("invalid");
    }
}
std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("expect not an empty expression");
    }
    Token token_meow = tokenizer->GetToken();
    if (auto* bracket = std::get_if<BracketToken>(&token_meow);
        bracket != nullptr && *bracket != BracketToken::OPEN) {
        throw SyntaxError("expected another bracket");
    }
    std::shared_ptr<Cell> root;
    std::shared_ptr<Cell> child;
    tokenizer->Next();
    while (!tokenizer->IsEnd()) {
        Token token_gaf = tokenizer->GetToken();
        if (auto* bracket = std::get_if<BracketToken>(&token_gaf);
            bracket != nullptr && *bracket == BracketToken::CLOSE) {
            if (child) {
                child->SetSecond(nullptr);
            }
            tokenizer->Next();
            return root;
        }
        if (auto* dot = std::get_if<DotToken>(&token_gaf); dot != nullptr) {
            if (root == nullptr) {
                throw SyntaxError("no dot in the beginning");
            }
            tokenizer->Next();
            if (tokenizer->IsEnd()) {
                throw SyntaxError("empty");
            }
            auto temp = Read(tokenizer);
            child->SetSecond(std::move(temp));
            if (tokenizer->IsEnd()) {
                throw SyntaxError("empty");
            }
            auto token = tokenizer->GetToken();
            if (auto* bracket = std::get_if<BracketToken>(&token);
                bracket == nullptr || *bracket != BracketToken::CLOSE) {
                throw SyntaxError("no closing bracket no end of the token");
            }
            tokenizer->Next();
            return root;
        }
        auto elem = Read(tokenizer);
        auto new_cell = std::make_shared<Cell>();
        new_cell->SetFirst(elem);
        if (!root) {
            root = new_cell;
        } else {
            child->SetSecond(new_cell);
        }
        child = new_cell;
    }
    throw SyntaxError("no closing bracket");
}