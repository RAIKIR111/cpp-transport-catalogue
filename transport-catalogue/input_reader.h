#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <cassert>
#include <algorithm>
#include <tuple>

namespace data_base {
using DataBaseMap = std::map<char, std::vector<std::string_view>>;
using ParsedNewStopInfo = std::tuple<std::string_view, double, double, std::vector<std::pair<std::string_view, int>>>;
using ParsedNewBusInfo = std::tuple<std::string_view, std::vector<std::string_view>, char>;

class InputReader {
public:
    InputReader(std::vector<std::string>& raw_data_base);

    std::tuple<std::vector<ParsedNewStopInfo>, std::vector<ParsedNewBusInfo>> ProccessDataBase();

private:
    DataBaseMap data_base_;
    std::vector<std::string> raw_data_;
};

namespace detail {
void DeleteEndSpaces(std::string_view& str);
}

}