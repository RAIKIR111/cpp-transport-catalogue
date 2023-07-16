#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <algorithm>
#include <tuple>

namespace transport_catalogue {
    class TransportCatalogue;
}

namespace data_base {
using ParsedNewStopInfo = std::tuple<std::string_view, double, double, std::vector<std::pair<std::string_view, int>>>;
using ParsedNewBusInfo = std::tuple<std::string_view, std::vector<std::string_view>, char>;

class InputReader {
public:
    InputReader(std::istream& input);

    std::tuple<std::vector<ParsedNewStopInfo>, std::vector<ParsedNewBusInfo>> ProccessRawDataBase() const;

    std::vector<std::pair<char, std::string_view>> ProccessRawRequests() const;

    class CharHasher {
    public:
        size_t operator()(const char& stopname) const {
            return hasher_(stopname);
        }

    private:
        std::hash<char> hasher_;
    };

    using DataBaseMap = std::unordered_map<char, std::vector<std::string_view>, CharHasher>;

private:
    DataBaseMap data_base_;
    std::vector<std::string> raw_data_;

    std::vector<std::pair<char, std::string_view>> requests_;
    std::vector<std::string> raw_data_base_;
};

namespace detail {
void DeleteEndSpaces(std::string_view& str);
}

}