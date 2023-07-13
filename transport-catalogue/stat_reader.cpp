#include "stat_reader.h"

using namespace std;
using namespace requests;

StatReader::StatReader(vector<string>& raw_requests) : raw_data_base_(move(raw_requests)) {
    for (const auto& item : raw_data_base_) {
        assert(item[0] == 'B' || item[0] == 'S'); //ASSERT
        auto it_first_not_space = find_if(find(item.begin(), item.end(), ' '), item.end(), [](const auto& entry) {
            return entry != ' ';
        });
        string_view temp_request = {&(*it_first_not_space), static_cast<string_view::size_type>(distance(it_first_not_space, item.end()))};
        requests_.push_back({item[0], temp_request});
    }
}

vector<pair<char, string_view>> StatReader::GetRequests() {
    return requests_;
}