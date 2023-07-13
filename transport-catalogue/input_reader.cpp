#include "input_reader.h"

using namespace std;
using namespace data_base;

InputReader::InputReader(vector<string>& raw_data_base) : raw_data_(move(raw_data_base)) {
    for (const auto& item : raw_data_) {
        assert(item[0] == 'S' || item[0] == 'B'); //ASSERT
        auto it_first_not_space = find_if(find(item.begin(), item.end(), ' '), item.end(), [](const auto& entry) {
            return entry != ' ';
        });
        data_base_[item[0]].push_back({&(*it_first_not_space), static_cast<string_view::size_type>(distance(it_first_not_space, item.end()))});
    }
}

const DataBaseMap& InputReader::GetDataBase() const {
    return data_base_;
}