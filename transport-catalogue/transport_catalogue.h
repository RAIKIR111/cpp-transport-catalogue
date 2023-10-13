#pragma once

#include <iostream>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <set>

#include "domain.h"

namespace transport_catalogue {
class TransportCatalogue {
public:    
    TransportCatalogue();

    void AddStop(const std::string_view name, const geo::Coordinates& coordinates);

    domain::Stop* FindStop(const std::string_view name) const;

    void AddBus(const std::string_view name, const std::vector<std::string_view>& entry_route, const bool is_roundtrip);

    domain::Bus* FindBus(const std::string_view name) const;

    void SetDistanceBetweenStops(const std::string_view first_name, const std::string_view second_name, const int distance);

    int GetDistanceBetweenStops(const std::string_view first_name, const std::string_view second_name) const;

    const std::unordered_set<domain::Bus*, domain::BusHasher>& FindBusesCrossingStop(domain::Stop* stop) const;

    const std::deque<domain::Stop>& GetStops() const;
    
    const std::deque<domain::Bus>& GetBuses() const;

    const std::unordered_map<domain::Stop*, std::unordered_set<domain::Bus*, domain::BusHasher>, domain::StopHasher>& GetStopToBuses() const;

    const std::unordered_map<std::string_view, bool, domain::StringViewHasher>& GetBusToIsroundtrip() const;

private:
    std::deque<domain::Stop> stops_;
    std::deque<domain::Bus> buses_;

    std::unordered_map<std::string_view, domain::Stop*, domain::StringViewHasher> stopname_to_stop_;
    std::unordered_map<std::string_view, domain::Bus*, domain::StringViewHasher> busname_to_bus_;

    std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, int, domain::PairStopsHasher> stops_to_distance_;
    std::unordered_map<domain::Stop*, std::unordered_set<domain::Bus*, domain::BusHasher>, domain::StopHasher> stop_to_buses_;

    std::unordered_map<std::string_view, bool, domain::StringViewHasher> busname_to_isroundtrip_;

    std::unordered_map<std::string_view, std::set<std::string_view>> stopname_to_stopnames_;
};

namespace detail {
void DeleteEndSpaces(std::string_view& str);
}

}