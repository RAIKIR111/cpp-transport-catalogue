#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <algorithm>
#include <sstream>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
   /* Реализуйте Node, используя std::variant */
    Node();

    Node(std::nullptr_t n);
    Node(bool boolean);
    Node(Array array);
    Node(Dict map);
    Node(int value);
    Node(std::string value);
    Node(double value);

    bool IsNull() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsString() const;
    bool IsArray() const;
    bool IsMap() const;
    bool IsBool() const;

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

    bool operator==(const Node& rhs) const {
        return rhs.smth_ == smth_;
    }

    bool operator!=(const Node& rhs) const {
        return !(*this == rhs);
    }

private:
    std::variant<int, double, std::string, bool, Array, Dict, std::nullptr_t> smth_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& rhs) const {
        return rhs.GetRoot() == GetRoot();
    }

    bool operator!=(const Document& rhs) const {
        return !(rhs == *this);
    }

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json