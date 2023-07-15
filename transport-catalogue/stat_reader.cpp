#include "stat_reader.h"

using namespace std;
using namespace requests;

StatReader::StatReader(istream& input) {
    string line;
    getline(input, line);
    int lines_number = stoi(line);
    raw_data_base_.reserve(lines_number);
    for (auto count = 0; count < lines_number; ++count) {
        getline(input, line);
        raw_data_base_.push_back(move(line));
    }

    for (const auto& item : raw_data_base_) {
        assert(item[0] == 'B' || item[0] == 'S'); //ASSERT
        auto it_first_not_space = find_if(find(item.begin(), item.end(), ' '), item.end(), [](const auto& entry) {
            return entry != ' ';
        });
        string_view temp_request = {&(*it_first_not_space), static_cast<string_view::size_type>(distance(it_first_not_space, item.end()))};
        requests_.push_back({item[0], temp_request});
    }
}

std::vector<std::pair<char, std::string_view>> StatReader::ProccessRequests() {
    return requests_;
}