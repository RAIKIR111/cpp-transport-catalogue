#pragma once

#include <memory>

#include "json.h"

namespace json {

class Builder {
    class BaseContext;
    class DictValueContext;
    class DictItemContext;
    class ArrayValueContext;

public:
    Builder() = default;

    DictValueContext Key(std::string);

    BaseContext Value(Node::Value);

    DictItemContext StartDict();

    ArrayValueContext StartArray();

    BaseContext EndDict();

    BaseContext EndArray();

    json::Node Build();

private:
    json::Node* root_ = nullptr;
    std::vector<json::Node*> nodes_stack_;

    // catching exceptions
    std::string arrays_and_dicts_;
    std::string last_called_method_;

    class BaseContext {
    public:
        BaseContext(Builder& builder)
            : builder_(builder) {
        }

        DictValueContext Key(std::string key) {
            return builder_.Key(std::move(key));
        }

        BaseContext Value(Node::Value val) {
            return builder_.Value(val);
        }

        BaseContext StartDict() {
            return builder_.StartDict();
        }

        ArrayValueContext StartArray() {
            return builder_.StartArray();
        }

        BaseContext EndDict() {
            return builder_.EndDict();
        }

        BaseContext EndArray() {
            return builder_.EndArray();
        }

        json::Node Build() {
            return builder_.Build();
        }

    private:
        Builder& builder_;
    };

    class DictValueContext : public BaseContext { // Dict
    public:
        DictValueContext(BaseContext base)
            : BaseContext(base) {
        }

        DictItemContext Value(Node::Value value) {
            return BaseContext::Value(std::move(value));
        }

        DictValueContext Key(std::string key) = delete;

        BaseContext EndDict() = delete;

        BaseContext EndArray() = delete;

        json::Node Build() = delete;
    };

    class DictItemContext : public BaseContext { // Simple Value
    public:
        DictItemContext(BaseContext base)
            : BaseContext(base) {
        }

        BaseContext Value(Node::Value val) = delete;

        BaseContext StartDict() = delete;

        BaseContext StartArray() = delete;

        BaseContext EndArray() = delete;

        json::Node Build() = delete;
    };

    class ArrayValueContext : public BaseContext { // Array
    public:
        ArrayValueContext(BaseContext base)
            : BaseContext(base) {
        }

        ArrayValueContext Value(Node::Value value) {
            return BaseContext::Value(std::move(value));
        }

        DictValueContext Key(std::string key) = delete;

        BaseContext EndDict() = delete;

        json::Node Build() = delete;

    };
};

} // namespace json