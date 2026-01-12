#pragma once

#include <memory>
#include <string>
#include <error.h>
#include "scope_fwd.h"
#include <vector>

class Object;

using ObjectPtr = std::shared_ptr<Object>;

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
    ObjectPtr Clone();
    virtual std::string Serialize() = 0;
    virtual ObjectPtr Evaluate(const std::shared_ptr<ScopesCollection>& scopes) = 0;
};

using IntType = int64_t;

class Number : public Object {
public:
    Number(IntType value) : value_(value) {
    }

    IntType GetValue() const {
        return value_;
    }

    std::string Serialize() override;
    ObjectPtr Evaluate(const std::shared_ptr<ScopesCollection>& scopes) override;

private:
    IntType value_;
};

class Symbol : public Object {
public:
    Symbol(std::string name) : name_(std::move(name)) {
    }

    const std::string& GetName() const {
        return name_;
    }

    std::string Serialize() override;
    ObjectPtr Evaluate(const std::shared_ptr<ScopesCollection>& scopes) override;

private:
    std::string name_;
};

class Boolean : public Object {
public:
    Boolean(bool value) : value_(value) {
    }

    bool GetValue() const {
        return value_;
    }

    std::string Serialize() override;
    ObjectPtr Evaluate(const std::shared_ptr<ScopesCollection>& scopes) override;

private:
    bool value_;
};

bool ToBool(const ObjectPtr& object);

class Cell : public Object {
public:
    std::shared_ptr<Object> GetFirst() const {
        return children_.first;
    }
    std::shared_ptr<Object> GetSecond() const {
        return children_.second;
    }

    void SetFirst(std::shared_ptr<Object> first) {
        children_.first = std::move(first);
    }
    void SetSecond(std::shared_ptr<Object> second) {
        children_.second = std::move(second);
    }

    std::string Serialize() override;
    ObjectPtr Evaluate(const std::shared_ptr<ScopesCollection>& scopes) override;

private:
    std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>> children_;
};

class IFunction : public Object {
public:
    std::vector<ObjectPtr> Call(const std::vector<ObjectPtr>& args,
                                const std::shared_ptr<ScopesCollection>& scopes);

    std::string Serialize() override;
    ObjectPtr Evaluate(const std::shared_ptr<ScopesCollection>& scopes) override;

protected:
    virtual std::vector<ObjectPtr> Prepare(const std::vector<ObjectPtr>& args,
                                           const std::shared_ptr<ScopesCollection>& scopes) = 0;
    virtual std::vector<ObjectPtr> DoCall(const std::vector<ObjectPtr>& args,
                                          const std::shared_ptr<ScopesCollection>& scopes) = 0;
};

class EvaluatingArgumentFunction : public IFunction {
    std::vector<ObjectPtr> Prepare(const std::vector<ObjectPtr>& args,
                                   const std::shared_ptr<ScopesCollection>& scopes) override;
};

class UnevaluatingArgumentFunction : public IFunction {
    std::vector<ObjectPtr> Prepare(const std::vector<ObjectPtr>& args,
                                   const std::shared_ptr<ScopesCollection>& scopes) override;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and conversion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    return As<T>(obj) != nullptr;
}

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename... Args>
std::shared_ptr<T> MakeNode(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}