#pragma once
#include <unordered_map>
#include <object.h>
#include "scope_fwd.h"
#include <vector>
#include <optional>
#include <error.h>

class Scope : public std::enable_shared_from_this<Scope> {
public:
    Scope(const std::unordered_map<std::string, std::shared_ptr<Object>>& objects,
          const std::shared_ptr<Scope>& parent)
        : objects_(objects), parent_(parent) {
    }

    std::shared_ptr<Object> Get(const std::string& key) {
        auto it = Find(key);
        return it.has_value() ? it.value()->second : nullptr;
    }

    bool ContainsInChain(const std::string& key) {
        return Find(key).has_value();
    }

    void Set(const std::string& key, const std::shared_ptr<Object>& value, bool in_current_scope) {
        if (in_current_scope) {
            UpdateValue(key, value);
            return;
        }

        auto it = Find(key);

        if (it.has_value()) {
            it.value()->second = value;
            return;
        }

        UpdateValue(key, value);
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Object>> objects_;

    std::shared_ptr<Scope> parent_;

    std::optional<std::unordered_map<std::string, std::shared_ptr<Object>>::iterator> Find(
        const std::string& key) {
        auto cur = shared_from_this();

        while (cur) {
            if (cur->objects_.contains(key)) {
                return cur->objects_.find(key);
            }
            cur = cur->parent_;
        }

        return std::nullopt;
    }

    void UpdateValue(const std::string& key, const std::shared_ptr<Object>& value) {
        objects_.insert_or_assign(key, value);
    }
};

class ScopesCollection {
public:
    ScopesCollection(const std::vector<std::shared_ptr<Scope>>& scopes) : scopes_(scopes) {
    }

    void AddScopes(const std::shared_ptr<ScopesCollection>& scopes) {
        if (scopes) {
            for (const auto& scope : scopes->scopes_) {
                scopes_.push_back(scope);
            }
        }
    }

    std::shared_ptr<Object> Get(const std::string& key) {
        for (auto& scope : scopes_) {
            auto res = scope->Get(key);
            if (res != nullptr) {
                return res;
            }
        }
        return nullptr;
    }

    void Set(const std::string& key, const std::shared_ptr<Object>& value, bool in_first_scope) {
        if (scopes_.empty()) {
            throw RuntimeError("no scopes to set");
        }

        if (in_first_scope) {
            UpdateValue(key, value, in_first_scope);
            return;
        }

        for (auto& scope : scopes_) {
            if (scope->ContainsInChain(key)) {
                scope->Set(key, value, false);
                return;
            }
        }
        UpdateValue(key, value, true);
    }

private:
    std::vector<std::shared_ptr<Scope>> scopes_;

    void UpdateValue(const std::string& key, const std::shared_ptr<Object>& value,
                     bool in_first_scope) {
        scopes_.front()->Set(key, value, in_first_scope);
    }
};