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
    class StringViewHasher {
    public:
        size_t operator()(const std::string_view& stopname) const {
            return hasher_(stopname);
        }

    private:
        std::hash<std::string_view> hasher_;
    };

    class PairStopsHasher {
    public:
        size_t operator()(const std::pair<domain::Stop*, domain::Stop*> stops) const {
            return hasher_(stops.first->name) + hasher_(stops.second->name) * 47;
        }
    private:
        std::hash<std::string_view> hasher_;
    };

    class StopHasher {
    public:
        size_t operator()(const domain::Stop* stop) const {
            return hasher_(stop->coordinates.lat) + hasher_(stop->coordinates.lng) * 47;
        }
    private:
        std::hash<double> hasher_;
    };

    class BusHasher {
    public:
        size_t operator()(const domain::Bus* bus) const {
            double dst = 0.0;
            int power = 1;
            for (const auto& item : bus->route) {
                dst += (item->coordinates.lat + item->coordinates.lng) * pow(27, power++);
            }
            return hasher_(dst);
        }
    private:
        std::hash<double> hasher_;
    };
    
    TransportCatalogue();

    void AddStop(const std::string_view name, const geo::Coordinates& coordinates);

    domain::Stop* FindStop(const std::string_view name) const;

    void AddBus(const std::string_view name, const std::vector<std::string_view>& entry_route, const bool is_roundtrip);

    domain::Bus* FindBus(const std::string_view name) const;

    void SetDistanceBetweenStops(const std::string_view first_name, const std::string_view second_name, const int distance);

    int GetDistanceBetweenStops(const std::string_view first_name, const std::string_view second_name) const;

    const std::unordered_set<domain::Bus*, BusHasher>& FindBusesCrossingStop(domain::Stop* stop) const;

    std::deque<domain::Stop> GetStops() const;
    
    std::deque<domain::Bus> GetBuses() const;

    std::vector<domain::Stop*> GetActiveStops() const;

    bool IsRoundTrip(const std::string_view bus_name) const;

private:
    std::deque<domain::Stop> stops_;
    std::deque<domain::Bus> buses_;

    std::unordered_map<std::string_view, domain::Stop*, StringViewHasher> stopname_to_stop_;
    std::unordered_map<std::string_view, domain::Bus*, StringViewHasher> busname_to_bus_;

    std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, int, PairStopsHasher> stops_to_distance_;
    std::unordered_map<domain::Stop*, std::unordered_set<domain::Bus*, BusHasher>, StopHasher> stop_to_buses_;

    std::unordered_map<domain::Bus*, bool, BusHasher> bus_to_isroundtrip_;
};

namespace detail {
void DeleteEndSpaces(std::string_view& str);
}

}