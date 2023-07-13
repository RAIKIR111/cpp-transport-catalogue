#include "transport_catalogue.h"

using namespace std;
using namespace transport_catalogue;

TransportCatalogue::TransportCatalogue(istream& input) {
    string line;
    getline(input, line);
    size_t lines_number = stoi(line);
    vector<string> input_reader_vector(lines_number);
    for (auto count = 0; count < lines_number; ++count) {
        getline(input, line);
        input_reader_vector[count] = move(line);
    }
    assert(input_reader_ptr_ == nullptr); // ASSERT
    input_reader_ptr_ = new data_base::InputReader(input_reader_vector);

    getline(input, line);
    lines_number = stoi(line);
    vector<string> stat_reader_vector(lines_number);
    for (auto count = 0; count < lines_number; ++count) {
        getline(input, line);
        stat_reader_vector[count] = move(line);
    }
    assert(stat_reader_ptr_ == nullptr); // ASSERT
    stat_reader_ptr_ = new requests::StatReader(stat_reader_vector);
}

void TransportCatalogue::ProccessDataBase() {
    const data_base::DataBaseMap raw_data_base = input_reader_ptr_->GetDataBase();
    if (raw_data_base.count('S')) {
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

            AddStop(main_stop_name, geo::Coordinates{stop_longitude, stop_latitude});

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
                    SetDistanceBetweenStops(main_stop_name, stop_name, distance);
                }
            }
        }
    }
    if (raw_data_base.count('B')) {
        for (const auto& item : raw_data_base.at('B')) {
            auto it_last_digit = find(item.begin(), item.end(), ':');
            string_view bus_name{&(*item.begin()), static_cast<string_view::size_type>(distance(item.begin(), it_last_digit))};
            detail::DeleteEndSpaces(bus_name);
            // bus name is ready

            char sym = (find(item.begin(), item.end(), '>') != item.end()) ? '>' : '-';
            vector<Stop*> bus_route;
            for (auto counter = 0; counter < count(item.begin(), item.end(), sym) + 1; ++counter) {
                auto it_1 = find_if(++it_last_digit, item.end(), [&sym](const auto& entry) {
                    return entry != ' ' && entry != ':' && entry != sym;
                });
                auto it_2 = find(it_1, item.end(), sym);
                string_view temp_main_stop_name{&(*it_1), static_cast<string_view::size_type>(distance(it_1, it_2))};
                detail::DeleteEndSpaces(temp_main_stop_name);

                it_last_digit = it_2;
                bus_route.push_back(stopname_to_stop_.at(temp_main_stop_name));
            }
            // bus route is ready

            AddBus(bus_name, bus_route, sym);
        }
    }
}

void TransportCatalogue::ProccessRequests() {
    vector<pair<char, string_view>> requests = stat_reader_ptr_->GetRequests();
    for (const auto& [type, request] : requests) {
        if (type == 'B') {
            if (!busname_to_bus_.count(request)) {
                cout << "Bus "s << request << ": not found"s << endl;
                continue;
            }

            Bus* bus_ptr = busname_to_bus_.at(request);

            unordered_set<string_view, StringViewHasher> unique_stops;
            int route_length = 0;
            double geo_length = 0;
            for (auto counter = 0; counter < bus_ptr->route.size() - 1; ++counter) {
                route_length += GetDistanceBetweenStops(bus_ptr->route.at(counter)->name, bus_ptr->route.at(counter + 1)->name);
                geo_length += geo::ComputeDistance(bus_ptr->route.at(counter)->coordinates, bus_ptr->route.at(counter + 1)->coordinates);
                unique_stops.insert(bus_ptr->route.at(counter)->name);
            }
            unique_stops.insert(bus_ptr->route.at(bus_ptr->route.size() - 1)->name);
            // route length is ready
            // unique stops is ready

            double curvature = double(route_length) / geo_length;
            // curvature is ready

            cout << "Bus "s << bus_ptr->name << ": "s << bus_ptr->route.size() << " stops on route, "s << unique_stops.size() << " unique stops, "s << route_length << " route length, "s << curvature << " curvature"s << endl;
        }
        else if (type == 'S') {
            if (!stopname_to_stop_.count(request)) {
                cout << "Stop " << request << ": not found" << endl;
                continue;
            }

            if (!stop_to_buses_.count(stopname_to_stop_.at(request))) {
                cout << "Stop " << request << ": no buses" << endl;
                continue;
            }
            
            set<string_view> temp_set_buses;
            for (const auto& item : stop_to_buses_.at(stopname_to_stop_.at(request))) {
                temp_set_buses.insert(item->name);
            }
            cout << "Stop " << request << ": buses";
            for (const auto& item : temp_set_buses) {
                cout << " " << item;
            }
            cout << endl;
        }
    }
}

void TransportCatalogue::AddStop(const string_view name, const geo::Coordinates& coordinates = geo::Coordinates{}) {
    if (stopname_to_stop_.count(name)) {
        Stop* stop = stopname_to_stop_.at(name);
        stop->coordinates = coordinates;
    }
    else {
        stops_.push_back({name, coordinates});
        stopname_to_stop_[name] = &(stops_.back());
    }
}

TransportCatalogue::Stop* TransportCatalogue::FindStop(const string_view name) {
    return stopname_to_stop_.at(name);
}

void TransportCatalogue::AddBus(const string_view name, vector<Stop*>& bus_route, const char sym) {
    if (sym == '-') {
        auto bus_route_copy = bus_route;
        bus_route_copy.pop_back();
        reverse(bus_route_copy.begin(), bus_route_copy.end());
        bus_route.insert(bus_route.end(), bus_route_copy.begin(), bus_route_copy.end());
    }

    buses_.push_back(Bus{name, bus_route});
    busname_to_bus_[name] = &buses_.back();
    for (const auto& stop : bus_route) {
        stop_to_buses_[stop].push_back(&buses_.back());
    }
}

TransportCatalogue::Bus* TransportCatalogue::FindBus(const string_view name) {
    return busname_to_bus_.at(name);
}

void TransportCatalogue::SetDistanceBetweenStops(const string_view first_name, const string_view second_name, const int distance) {
    if (!stopname_to_stop_.count(second_name)) {
        AddStop(second_name);
    }
    stops_to_distance_[{stopname_to_stop_.at(first_name), stopname_to_stop_.at(second_name)}] = distance;
    if (!stops_to_distance_.count({stopname_to_stop_.at(second_name), stopname_to_stop_.at(first_name)})) {
        stops_to_distance_[{stopname_to_stop_.at(second_name), stopname_to_stop_.at(first_name)}] = distance;
    }
}

int TransportCatalogue::GetDistanceBetweenStops(const string_view first_name, const string_view second_name) {
    return stops_to_distance_.at({stopname_to_stop_.at(first_name), stopname_to_stop_.at(second_name)});
}

void transport_catalogue::detail::DeleteEndSpaces(string_view& str) {
    if (str.at(str.size() - 1) != ' ') {
        return;
    }
    size_t last_non_space = str.find_last_not_of(" ");
    if (last_non_space != string::npos) {
        str = str.substr(0, last_non_space + 1);
    }
}