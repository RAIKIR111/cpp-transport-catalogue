#include "transport_catalogue.h"

using namespace std;
using namespace transport_catalogue;
using namespace domain;

TransportCatalogue::TransportCatalogue() {
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
    stop_to_buses_[stopname_to_stop_.at(name)];
}

Stop* TransportCatalogue::FindStop(const string_view name) const {
    if (!stopname_to_stop_.count(name)) {
        return nullptr;
    }
    return stopname_to_stop_.at(name);
}

void TransportCatalogue::AddBus(const string_view name, const vector<string_view>& entry_route, const bool is_roundtrip) {
    vector<Stop*> dst_route;
    if (is_roundtrip == false) {
        dst_route.reserve((entry_route.size() * 2) - 1);
        for (auto counter = 0; counter < entry_route.size(); ++counter) {
            const string_view& stopname = entry_route.at(counter);
            dst_route.push_back(stopname_to_stop_.at(stopname));
            if (counter != entry_route.size() - 1) {
                stopname_to_stopnames_[stopname].insert(entry_route.at(counter + 1));
                stopname_to_stopnames_[entry_route.at(counter + 1)].insert(entry_route.at(counter));
            }
        }
        for (int counter = entry_route.size() - 2; counter >= 0; counter--) {
            dst_route.push_back(stopname_to_stop_.at(entry_route.at(counter)));
        }
    }
    else {
        dst_route.reserve(entry_route.size());
        for (auto counter = 0; counter < entry_route.size(); ++counter) {
            const string_view& stopname = entry_route.at(counter);
            dst_route.push_back(stopname_to_stop_.at(stopname));
            if (counter != entry_route.size() - 1) {
                stopname_to_stopnames_[stopname].insert(entry_route.at(counter + 1));
            }
        }
    }

    buses_.push_back(Bus{name, dst_route});
    busname_to_bus_[name] = &buses_.back();
    for (const auto& stop : dst_route) {
        stop_to_buses_[stop].insert(&buses_.back());
    }
    busname_to_isroundtrip_[buses_.back().name] = is_roundtrip;
}

Bus* TransportCatalogue::FindBus(const string_view name) const {
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

const std::unordered_set<Bus*, domain::BusHasher>& TransportCatalogue::FindBusesCrossingStop(Stop* stop) const {
    return stop_to_buses_.at(stop);
}

const std::deque<domain::Stop>& TransportCatalogue::GetStops() const {
    return stops_;
}

const std::deque<domain::Bus>& TransportCatalogue::GetBuses() const {
    return buses_;
}

const std::unordered_map<domain::Stop*, std::unordered_set<domain::Bus*, domain::BusHasher>, domain::StopHasher>& TransportCatalogue::GetStopToBuses() const {
    return stop_to_buses_;
}

const std::unordered_map<std::string_view, bool, domain::StringViewHasher>& TransportCatalogue::GetBusToIsroundtrip() const {
    return busname_to_isroundtrip_;
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