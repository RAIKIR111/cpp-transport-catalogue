#include "input_reader.h"
#include "transport_catalogue.h"

using namespace std;
using namespace data_base;

InputReader::InputReader(istream& input) {
    string line;
    getline(input, line);
    size_t lines_number = stoi(line);
    raw_data_.reserve(lines_number);
    for (auto count = 0; count < lines_number; ++count) {
        getline(input, line);
        raw_data_.push_back(move(line));
    }

    for (const auto& item : raw_data_) {
        assert(item[0] == 'S' || item[0] == 'B'); //ASSERT
        auto it_first_not_space = find_if(find(item.begin(), item.end(), ' '), item.end(), [](const auto& entry) {
            return entry != ' ';
        });
        data_base_[item[0]].push_back({&(*it_first_not_space), static_cast<string_view::size_type>(distance(it_first_not_space, item.end()))});
    }

    getline(input, line);
    lines_number = stoi(line);
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

std::tuple<std::vector<ParsedNewStopInfo>, std::vector<ParsedNewBusInfo>> InputReader::ProccessRawDataBase() const {
    const DataBaseMap& raw_data_base = data_base_;
    vector<ParsedNewStopInfo> parsed_new_stop_info_vec;
    vector<ParsedNewBusInfo> parsed_new_bus_info_vec;
    if (raw_data_base.count('S')) {
        parsed_new_stop_info_vec.reserve(raw_data_base.at('S').size());
        for (const auto& item : raw_data_base.at('S')) {
            auto it_two_dots = find(item.begin(), item.end(), ':');
            string_view main_stop_name{&(*item.begin()), static_cast<string_view::size_type>(distance(item.begin(), it_two_dots))};
            detail::DeleteEndSpaces(main_stop_name);
            // stop name is ready

            size_t first_digit_index = item.find_first_of("0123456789", distance(item.begin(), it_two_dots));
            size_t dot_or_space_index = item.find_first_of(" ,", first_digit_index);
            double stop_longitude = stod(string{&item[first_digit_index], &item[dot_or_space_index]});
            // stop longitude is ready

            first_digit_index = item.find_first_of("0123456789", dot_or_space_index);
            double stop_latitude = stod(string{&item[first_digit_index], &item[item.size()]});
            // stop latitude is ready

            vector<pair<string_view, int>> stopnames_and_distances;
            dot_or_space_index = item.find(',', first_digit_index);
            if (dot_or_space_index != string::npos) {
                auto number_of_dots = count(item.begin() + dot_or_space_index, item.end(), ',');
                for (auto counter = 0; counter < number_of_dots; ++counter) {
                    first_digit_index = item.find_first_of("0123456789", dot_or_space_index);
                    dot_or_space_index = item.find('m', first_digit_index);
                    int distance = stoi(string{&item[first_digit_index], &item[dot_or_space_index]});
                    first_digit_index = item.find("to", dot_or_space_index);
                    first_digit_index += 2;
                    first_digit_index = item.find_first_not_of(' ', first_digit_index);
                    dot_or_space_index = (counter != number_of_dots - 1) ? item.find(',', first_digit_index) : item.size();
                    string_view stop_name{&item[first_digit_index], dot_or_space_index - first_digit_index};
                    detail::DeleteEndSpaces(stop_name);
                    // distance between stops is ready
                    stopnames_and_distances.push_back({stop_name, distance});
                }
            }

            parsed_new_stop_info_vec.push_back(make_tuple(main_stop_name, stop_longitude, stop_latitude, stopnames_and_distances));
        }
    }
    if (raw_data_base.count('B')) {
        parsed_new_bus_info_vec.reserve(raw_data_base.at('B').size());
        for (const auto& item : raw_data_base.at('B')) {
            auto it_last_digit = find(item.begin(), item.end(), ':');
            string_view bus_name{&(*item.begin()), static_cast<string_view::size_type>(distance(item.begin(), it_last_digit))};
            detail::DeleteEndSpaces(bus_name);
            // bus name is ready

            char sym = (find(item.begin(), item.end(), '>') != item.end()) ? '>' : '-';
            vector<string_view> stops_names;
            for (auto counter = 0; counter < count(item.begin(), item.end(), sym) + 1; ++counter) {
                auto it_1 = find_if(++it_last_digit, item.end(), [&sym](const auto& entry) {
                    return entry != ' ' && entry != ':' && entry != sym;
                });
                auto it_2 = find(it_1, item.end(), sym);
                string_view temp_main_stop_name{&(*it_1), static_cast<string_view::size_type>(distance(it_1, it_2))};
                detail::DeleteEndSpaces(temp_main_stop_name);

                it_last_digit = it_2;
                stops_names.push_back(temp_main_stop_name);
            }
            // bus route is ready

            parsed_new_bus_info_vec.push_back(make_tuple(bus_name, stops_names, sym));
        }
    }
    return make_tuple(parsed_new_stop_info_vec, parsed_new_bus_info_vec);
}

std::vector<std::pair<char, std::string_view>> InputReader::ProccessRawRequests() const {
    return requests_;
}

void data_base::detail::DeleteEndSpaces(string_view& str) {
    if (str.at(str.size() - 1) != ' ') {
        return;
    }
    size_t last_non_space = str.find_last_not_of(" ");
    if (last_non_space != string::npos) {
        str = str.substr(0, last_non_space + 1);
    }
}