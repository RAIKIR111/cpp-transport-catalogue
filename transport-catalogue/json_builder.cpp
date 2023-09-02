#include "json_builder.h"

using namespace json;
using namespace std;

Builder::DictValueContext Builder::Key(std::string key) {
    if (root_ != nullptr && !root_->IsArray() && !root_->IsDict()) {
        throw logic_error(""s);
    }

    if (last_called_method_ == "Key"s || arrays_and_dicts_.size() == 0 || arrays_and_dicts_.back() != '{') {
        throw logic_error(""s);
    }

    Node* temp_node = new Node();
    temp_node->GetNonConstValue() = key;
    nodes_stack_.push_back(temp_node);

    last_called_method_ = "Key"s;

    return DictValueContext(*this);
}

Builder::BaseContext Builder::Value(Node::Value val) {
    if (root_ != nullptr && !root_->IsArray() && !root_->IsDict()) {
        throw logic_error(""s);
    }

    if (!(last_called_method_.size() == 0 || last_called_method_ == "Key" || arrays_and_dicts_.back() == '[')) {
        throw logic_error(""s);
    }

    if (root_ == nullptr) {
        root_ = new Node{};

        root_->GetNonConstValue() = val;

        return BaseContext(*this);
    }
    else {
        Node* temp_node = new Node();
        temp_node->GetNonConstValue() = val;
        nodes_stack_.push_back(temp_node);
    }

    last_called_method_ = "Value"s;

    if (arrays_and_dicts_.back() == '[') {
        return ArrayValueContext(*this);
    }
    else if (arrays_and_dicts_.back() == '{') {
        return DictValueContext(*this);
    }

    return BaseContext(*this);
}

Builder::DictItemContext Builder::StartDict() {
    if (root_ != nullptr && !root_->IsArray() && !root_->IsDict()) {
        throw logic_error(""s);
    }

    if (!(last_called_method_.size() == 0 || last_called_method_ == "Key"s || arrays_and_dicts_.back() == '[')) {
        throw logic_error(""s);
    }

    arrays_and_dicts_.push_back('{');

    if (root_ == nullptr) {
        root_ = new Node{};

        root_->GetNonConstValue() = Dict();
        nodes_stack_.push_back(root_);
    }
    else {
        Node* temp_node = new Node();
        temp_node->GetNonConstValue() = Dict();
        nodes_stack_.push_back(temp_node);
    }

    last_called_method_ = "StartDict"s;

    DictItemContext kek{*this};

    return DictItemContext(*this);
}

Builder::ArrayValueContext Builder::StartArray() {
    if (root_ != nullptr && !root_->IsArray() && !root_->IsDict()) {
        throw logic_error(""s);
    }

    if (!(last_called_method_.size() == 0 || last_called_method_ == "Key" || arrays_and_dicts_.back() == '[')) {
        throw logic_error(""s);
    }

    arrays_and_dicts_.push_back('[');

    if (root_ == nullptr) {
        root_ = new Node{};

        root_->GetNonConstValue() = Array();
        nodes_stack_.push_back(root_);
    }
    else {
        Node* temp_node = new Node();
        temp_node->GetNonConstValue() = Array();
        nodes_stack_.push_back(temp_node);
    }

    last_called_method_ = "StartArray"s;

    return ArrayValueContext(*this);
}

Builder::BaseContext Builder::EndDict() {
    if (root_ != nullptr && !root_->IsArray() && !root_->IsDict()) {
        throw logic_error(""s);
    }

    if (arrays_and_dicts_.back() == '[') {
        throw logic_error(""s);
    }

    if (last_called_method_ == "Key"s) {
        throw logic_error(""s);
    }
    
    arrays_and_dicts_.erase(arrays_and_dicts_.size() - 1);

    vector<Node*>::iterator dst_it;
    for (auto it = nodes_stack_.rbegin(); it != nodes_stack_.rend(); ++it) {
        if ((*it)->IsDict() && (*it)->AsDict().size() == 0) {
            dst_it = it.base();
            break;
        }
    }

    auto array_it = dst_it - 1;
    
    for (auto it = dst_it; it != nodes_stack_.end(); it++) {
        const string& temp_str = (*it)->AsString();
        ++it;
        (*array_it)->AsNonConstDict()[temp_str] = (**(it));
    }
    nodes_stack_.erase(dst_it, nodes_stack_.end());

    last_called_method_ = "EndDict"s;

    return DictValueContext(*this);
}

Builder::BaseContext Builder::EndArray() {
    if (root_ != nullptr && !root_->IsArray() && !root_->IsDict()) {
        throw logic_error(""s);
    }

    if (arrays_and_dicts_.back() == '{') {
        throw logic_error(""s);
    }

    arrays_and_dicts_.erase(arrays_and_dicts_.size() - 1);
    
    vector<Node*>::iterator dst_it;
    for (auto it = nodes_stack_.rbegin(); it != nodes_stack_.rend(); ++it) {
        if ((*it)->IsArray() && (*it)->AsArray().size() == 0) {
            dst_it = it.base();
            break;
        }
    }

    auto array_it = dst_it - 1;
    
    for (auto it = dst_it; it != nodes_stack_.end(); ++it) {
        (*array_it)->AsNonConstArray().emplace_back(**it);
    }
    nodes_stack_.erase(dst_it, nodes_stack_.end());

    last_called_method_ = "EndArray"s;

    return ArrayValueContext(*this);
}

Node Builder::Build() {
    if (root_ == nullptr || arrays_and_dicts_.size() != 0) {
        throw logic_error(""s);
    }

    last_called_method_ = "Build"s;

    return *root_;
}