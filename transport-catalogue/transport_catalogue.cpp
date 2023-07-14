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
    auto ready_info = input_reader_ptr_->ProccessDataBase();
    for (const auto& parsed_stop_info : get<0>(ready_info)) {
        string_view main_stopname = get<0>(parsed_stop_info);
        geo::Coordinates coordinates{get<1>(parsed_stop_info), get<2>(parsed_stop_info)};
        AddStop(main_stopname, coordinates);
        for (const auto& [stopname, distance] : get<3>(parsed_stop_info)) {
            SetDistanceBetweenStops(main_stopname, stopname, distance);
        }
    }
    for (const auto& parsed_bus_info : get<1>(ready_info)) {
        string_view busname = get<0>(parsed_bus_info);
        vector<string_view> route = get<1>(parsed_bus_info);
        char sym = get<2>(parsed_bus_info);
        AddBus(busname, route, sym);
    }

    getline(input, line);
    lines_number = stoi(line);
    vector<string> stat_reader_vector(lines_number);
    for (auto count = 0; count < lines_number; ++count) {
        getline(input, line);
        stat_reader_vector[count] = move(line);
    }
    assert(stat_reader_ptr_ == nullptr); // ASSERT
    stat_reader_ptr_ = new requests::StatReader(stat_reader_vector);
    auto requests = stat_reader_ptr_->ProccessRequests();
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

TransportCatalogue::~TransportCatalogue() {
    delete input_reader_ptr_;
    delete stat_reader_ptr_;
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

void TransportCatalogue::AddBus(const string_view name, const vector<string_view>& entry_route, const char sym) {
    vector<Stop*> dst_route;
    if (sym == '-') {
        dst_route.reserve(entry_route.size() * 2 - 1);
        for (const auto& stopname : entry_route) {
            dst_route.push_back(stopname_to_stop_.at(stopname));
        }
        for (int counter = entry_route.size() - 2; counter >= 0; counter--) {
            dst_route.push_back(stopname_to_stop_.at(entry_route.at(counter)));
        }
    }
    else {
        dst_route.reserve(entry_route.size());
        for (const auto& stopname : entry_route) {
            dst_route.push_back(stopname_to_stop_.at(stopname));
        }
    }

    buses_.push_back(Bus{name, dst_route});
    busname_to_bus_[name] = &buses_.back();
    for (const auto& stop : dst_route) {
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