#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <cassert>
#include <algorithm>

namespace data_base {
using DataBaseMap = std::map<char, std::vector<std::string_view>>;

class InputReader {
public:
    InputReader(std::vector<std::string>& raw_data_base);

    const DataBaseMap& GetDataBase() const;

private:
    DataBaseMap data_base_;
    std::vector<std::string> raw_data_;
};
}