#include "transport_catalogue.h"

using namespace std;
using namespace transport_catalogue;

TransportCatalogue::TransportCatalogue() {
}

void TransportCatalogue::InitializeTransportCatalogue(istream& input) {
    data_base::InputReader* input_reader_ptr = new data_base::InputReader(input);
    requests::StatReader* stat_reader_ptr = new requests::StatReader(this, input_reader_ptr);
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

TransportCatalogue::Stop* TransportCatalogue::FindStop(const string_view name) const {
    if (!stopname_to_stop_.count(name)) {
        return nullptr;
    }
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

TransportCatalogue::Bus* TransportCatalogue::FindBus(const string_view name) const {
    if (!busname_to_bus_.count(name)) {
        return nullptr;
    }
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

int TransportCatalogue::GetDistanceBetweenStops(const string_view first_name, const string_view second_name) const {
    return stops_to_distance_.at({stopname_to_stop_.at(first_name), stopname_to_stop_.at(second_name)});
}

std::vector<TransportCatalogue::Bus*> TransportCatalogue::FindBusesCrossingStop(Stop* stop) const {
    if (!stop_to_buses_.count(stop)) {
        return vector<TransportCatalogue::Bus*>();
    }
    return stop_to_buses_.at(stop);
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