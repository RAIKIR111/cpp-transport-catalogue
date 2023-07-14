#pragma once

#include <iostream>
#include <cassert>
#include <algorithm>
#include <vector>
#include <string_view>

namespace requests {

class StatReader {
public:
    StatReader(std::vector<std::string>& raw_requests);

    std::vector<std::pair<char, std::string_view>> ProccessRequests();

private:
    std::vector<std::pair<char, std::string_view>> requests_;
    std::vector<std::string> raw_data_base_;
};
}