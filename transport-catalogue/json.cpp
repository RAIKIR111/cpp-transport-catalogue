#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadString(istream& input);

Node LoadArray(istream& input) {
    Array result;
    input.ignore(1);

    bool break_flag = false;

    string line;
    char c;
    while ((c = input.get()) && c != ']' && c != EOF) {
        if (c == '[' || c == '{') {
            const char main_bracket = c;

            int open_brackets = 1;
            int close_brackets = 0;
            line.push_back(main_bracket);
            while (open_brackets != close_brackets) {
                c = input.get();
                line.push_back(c);
                if (c == main_bracket) {
                    open_brackets += 1;
                }
                if (c == ((main_bracket == '[') ? ']' : '}')) {
                    close_brackets += 1;
                }
            }
            istringstream iss{line};
            line.clear();

            result.push_back(LoadNode(iss));
            while ((c = input.get())) {
                if (c == ',') {
                    break;
                }
                else if (c == ']') {
                    break_flag = true;
                    break;
                }
            }

            if (break_flag == true) {
                break;
            }
        }
        else if (c == ',') {
            istringstream iss{line};
            line.clear();

            result.push_back(LoadNode(iss));
        }
        else {
            line.push_back(c);
        }
    }

    if (c != ']') {
        throw ParsingError("Parsing Error!");
    }
    
    const string chars_to_skip_no_space = " \n\r\t\\";
    if ((line.find_first_not_of(chars_to_skip_no_space) == string::npos) && (result.size() == 0)) {
        return Node(move(result));
    }

    if (line.size() != 0) {
        istringstream iss{line};
        result.push_back(LoadNode(iss));
    }
    
    return Node(move(result));
}

Node LoadDict(istream& input) {
    Dict result;
    input.ignore(1);

    bool break_flag = false;
    bool two_dots_flag = false;

    string line;
    string key;
    char c;
    while ((c = input.get()) && c != '}' && c != EOF) {
        if ((c == '[' || c == '{') && two_dots_flag == true) {
            const char main_bracket = c;

            int open_brackets = 1;
            int close_brackets = 0;
            line.push_back(main_bracket);
            while (open_brackets != close_brackets) {
                c = input.get();
                line.push_back(c);
                if (c == main_bracket) {
                    open_brackets += 1;
                }
                if (c == ((main_bracket == '[') ? ']' : '}')) {
                    close_brackets += 1;
                }
            }
            istringstream iss{line};
            istringstream iss_key{key};
            line.clear();

            result[LoadNode(iss_key).AsString()] = LoadNode(iss);
            while ((c = input.get())) {
                if (c == ',') {
                    break;
                }
                else if (c == '}') {
                    break_flag = true;
                    break;
                }
            }

            two_dots_flag = false;

            if (break_flag == true) {
                break;
            }
        }
        else if (c == ':') {
            two_dots_flag = true;
            key = line;
            line.clear();
        }
        else if (c == ',' && two_dots_flag == true) {
            istringstream iss_key{key};
            istringstream iss_val{line};

            result[LoadNode(iss_key).AsString()] = LoadNode(iss_val);
            line.clear();

            two_dots_flag = false;
        }
        else {
            line.push_back(c);
        }
    }

    if (c != '}') {
        throw ParsingError("Parsing Error!");
    }

    const string chars_to_skip_no_space = " \n\r\t\\";
    if ((line.find_first_not_of(chars_to_skip_no_space) == string::npos) && (result.size() == 0)) {
        return Node(move(result));
    }

    if (line.size() != 0) {
        istringstream iss_key{key};
        istringstream iss_val{line};
        result[LoadNode(iss_key).AsString()] = LoadNode(iss_val);
    }

    return Node(move(result));
}

Node LoadInt(istream& input) {
    bool minus_flag = false;
    if (input.peek() == '-') {
        input.get();
        minus_flag = true;
    }

    int result = 0;
    while (isdigit(input.peek())) {
        result *= 10;
        result += input.get() - '0';
    }

    if (minus_flag == false) {
        return Node(result);
    }

    return (minus_flag == false) ? Node(result) : Node(-result);
}

Node LoadDouble(istream& input) {
    string temp_str;
    getline(input, temp_str);
    return Node(stod(temp_str));
}

Node LoadString(istream& input) {
    string line;
    char c;
    input.ignore(1);

    while (input.get(c)) {
        line += c;
    }
    size_t last_quotes = line.find_last_of("\"");
    if (last_quotes == string::npos) {
        throw ParsingError("Parsing Error!");
    }
    line = line.substr(0, last_quotes);

    if (line.size() <= 1) {
        return Node(move(line));
    }

    string dst_line;
    for (size_t counter = 0; counter < line.size() - 1; counter++) {
        if (line.at(counter) == '\\' && line.at(counter + 1) == 'r') {
            dst_line.push_back('\r');
            ++counter;
            continue;
        }
        if (line.at(counter) == '\\' && line.at(counter + 1) == 'n') {
            dst_line.push_back('\n');
            ++counter;
            continue;
        }
        if (line.at(counter) == '\\' && line.at(counter + 1) == '\"') {
            dst_line.push_back('\"');
            ++counter;
            continue;
        }
        if (line.at(counter) == '\\' && line.at(counter + 1) == '\\') {
            dst_line.push_back('\\');
            ++counter;
            continue;
        }
        if (line.at(counter) == '\\' && line.at(counter + 1) == 't') {
            dst_line.push_back('\t');
            ++counter;
            continue;
        }
        dst_line.push_back(line.at(counter));
    }
    if ((line.at(line.size() - 2) == '\\' && line.at(line.size() - 1) == 'r') || 
        (line.at(line.size() - 2) == '\\' && line.at(line.size() - 1) == 'n') ||
        (line.at(line.size() - 2) == '\\' && line.at(line.size() - 1) == '\"') ||
        (line.at(line.size() - 2) == '\\' && line.at(line.size() - 1) == '\\') ||
        (line.at(line.size() - 2) == '\\' && line.at(line.size() - 1) == 't')) {

        return Node(move(dst_line));
    }
    dst_line.push_back(line.at(line.size() - 1));

    return Node(move(dst_line));
}

Node LoadBool(istream& input) {
    char c;
    string line;
    while (input.get(c)) {
        line += c;
    }
    if (line == "false") {
        return Node(false);
    }
    return Node(true);
}

Node LoadNode(istream& input) {
    auto check_if_int = [](const string& entry) {
        string number_syms = "1234567890";
        if (entry.at(0) != '-' && number_syms.find(entry.at(0)) == string::npos) {
            return false;
        }
        for (size_t counter = 1; counter < entry.size(); ++counter) {
            if (number_syms.find(entry.at(counter)) == string::npos) {
                return false;
            }
        }
        return true;
    };

    auto check_if_double = [](const string& entry) {
        string digits = "1234567890";
        string number_syms = "1234567890.eE-+";
        if (entry.at(0) != '-' && digits.find(entry.at(0)) == string::npos) {
            return false;
        }
        bool dot_flag = false;
        bool e_flag = false;
        bool E_flag = false;
        bool minus_flag = false;
        bool plus_flag = false;
        for (size_t counter = 1; counter < entry.size(); ++counter) {
            if (number_syms.find(entry.at(counter)) == string::npos) {
                return false;
            }
            if (entry.at(counter) == '.') {
                if (dot_flag == false) {
                    dot_flag = true;
                }
                else {
                    return false;
                }
            }
            if (entry.at(counter) == 'e') {
                if (e_flag == false) {
                    e_flag = true;
                }
                else {
                    return false;
                }
            }
            if (entry.at(counter) == 'E') {
                if (E_flag == false) {
                    E_flag = true;
                }
                else {
                    return false;
                }
            }
            if (entry.at(counter) == '-') {
                if (minus_flag == false) {
                    minus_flag = true;
                }
                else {
                    return false;
                }
            }
            if (entry.at(counter) == '+') {
                if (plus_flag == false) {
                    plus_flag = true;
                }
                else {
                    return false;
                }
            }
        }
        return true;
    };
    
    const string chars_to_skip_no_space = " \n\r\t\\";

    // delete begin skip-chars
    while (input.peek() != EOF) {
        char next_char = input.peek();
        if (chars_to_skip_no_space.find(next_char) != string::npos) {
            input.ignore(1);
        }
        else {
            break;
        }
    }

    char c = input.peek();
    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '\"') {
        return LoadString(input);
    } else {
        char c;
        string temp_line;
        while (input.get(c)) {
            temp_line.push_back(c);
        }
        
        // delete end skip-chars
        string chars_to_skip = " \n\r\"\\\t";
        size_t last_good_char = temp_line.find_last_not_of(chars_to_skip);
        temp_line = temp_line.substr(0, last_good_char + 1);

        // delete inner skip-chars
        string line;
        copy_if(temp_line.begin(), temp_line.end(), back_inserter(line), [&chars_to_skip](const auto& entry) {
            return (chars_to_skip.find(entry) == string::npos);
        });

        // null
        if (line == "null") {
            return Node();
        }

        istringstream iss(line);
        // int
        if (check_if_int(line)) {
            return LoadInt(iss);
        }

        // double
        if (check_if_double(line)) {
            return LoadDouble(iss);
        }

        // bool
        if (line == "false" || line == "true") {
            return LoadBool(iss);
        }
    }

    throw ParsingError("Parsing Error!");
}

}  // namespace

Node::Node()
    : Node(nullptr) {
}

Node::Node(nullptr_t n)
    : smth_(n) {
}

Node::Node(bool boolean)
    : smth_(move(boolean)) {
}

Node::Node(Array array)
    : smth_(move(array)) {
}

Node::Node(Dict map)
    : smth_(move(map)) {
}

Node::Node(int value)
    : smth_(value) {
}

Node::Node(string value)
    : smth_(move(value)) {
}

Node::Node(double value)
    : smth_(value) {
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(smth_);
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(smth_);
}

bool Node::IsDouble() const {
    return std::holds_alternative<double>(smth_) || std::holds_alternative<int>(smth_);
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(smth_);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(smth_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(smth_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(smth_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(smth_);
}

int Node::AsInt() const {
    if (holds_alternative<int>(smth_)) {
        return get<int>(smth_);
    }
    else {
        throw std::logic_error("incorrect type required!");
    }
}

bool Node::AsBool() const {
    if (holds_alternative<bool>(smth_)) {
        return get<bool>(smth_);
    }
    else {
        throw std::logic_error("incorrect type required!");
    } 
}

double Node::AsDouble() const {
    if (holds_alternative<double>(smth_) || holds_alternative<int>(smth_)) {
        if (holds_alternative<double>(smth_)) {
            return get<double>(smth_);
        }
        else 
            return get<int>(smth_);
    }
    else {
        throw std::logic_error("incorrect type required!");
    } 
}

const string& Node::AsString() const {
    if (holds_alternative<string>(smth_)) {
        return get<string>(smth_);
    }
    else {
        throw std::logic_error("incorrect type required!");
    }
}

const Array& Node::AsArray() const {
    if (holds_alternative<Array>(smth_)) {
        return get<Array>(smth_);
    }
    else {
        throw std::logic_error("incorrect type required!");
    } 
}

const Dict& Node::AsMap() const {
    if (holds_alternative<Dict>(smth_)) {
        return get<Dict>(smth_);
    }
    else {
        throw std::logic_error("incorrect type required!");
    } 
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    if (doc.GetRoot().IsNull()) {
        output << "null";
    }
    else if (doc.GetRoot().IsInt()) {
        output << to_string(doc.GetRoot().AsInt());
    }
    else if (doc.GetRoot().IsDouble()) {
        auto temp_str = to_string(doc.GetRoot().AsDouble());
        size_t first_not_zero = temp_str.find_last_not_of('0', temp_str.size() - 1);
        if (temp_str.find('.') == first_not_zero) {
            output << temp_str.substr(0, first_not_zero);
        }
        else {
            output << temp_str.substr(0, first_not_zero + 1);
        }
    }
    else if (doc.GetRoot().IsString()) {
        const string spec_chars = "\n\r\"\\\t";
        const auto temp_str = doc.GetRoot().AsString();

        output << "\"";
        for (size_t counter = 0; counter < temp_str.size(); ++counter) {
            if (temp_str.at(counter) == '\r') {
                output << "\\r";
            } else if (temp_str.at(counter) == '\n') {
                output << "\\n";
            } else if (temp_str.at(counter) == '\"') {
                output << "\\\"";
            } else if (temp_str.at(counter) == '\\' && (temp_str.size() - 1) > counter && temp_str.at(counter + 1) == 't') {
                output << "\t";
                ++counter;
            } else if (temp_str.at(counter) == '\\') {
                output << "\\\\";
            } else {
                output << temp_str.at(counter);
            }
        }
        output << "\"";
    }
    else if (doc.GetRoot().IsBool()) {
        const bool b = doc.GetRoot().AsBool();
        if (b == false) {
            output << "false";
        }
        else {
            output << "true";
        }
    }
    else if (doc.GetRoot().IsArray()) {
        bool comma_flag = false;
        output << "[";
        for (const auto& item : doc.GetRoot().AsArray()) {
            if (comma_flag == false) {
                comma_flag = true;
            }
            else {
                output << ",";
            }
            Print(Document(item), output);
        }
        output << "]";
    }
    else if (doc.GetRoot().IsMap()) {
        bool comma_flag = false;
        output << "{";
        for (const auto& [key, value] : doc.GetRoot().AsMap()) {
            if (comma_flag == false) {
                comma_flag = true;
            }
            else {
                output << ',';
            }
            output << " \"" << key << "\": ";
            Print(Document(value), output);
        }
        output << " }";
    }
    // Реализуйте функцию самостоятельно
}

}  // namespace json