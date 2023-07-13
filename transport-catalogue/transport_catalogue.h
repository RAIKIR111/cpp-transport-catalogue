#pragma once

#include <iostream>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <set>

#include "input_reader.h"
#include "stat_reader.h"
#include "geo.h"

namespace transport_catalogue {
class TransportCatalogue {
public:
    TransportCatalogue(std::istream& input);

    void ProccessDataBase();

    void ProccessRequests();

private:
    struct Stop {
        std::string_view name;
        geo::Coordinates coordinates;
    };

    struct Bus {
        std::string_view name;
        std::vector<Stop*> route;
    };

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
        size_t operator()(const std::pair<Stop*, Stop*> stops) const {
            return hasher_(stops.first->name) + hasher_(stops.second->name) * 47;
        }
    private:
        std::hash<std::string_view> hasher_;
    };

    class StopHasher {
    public:
        size_t operator()(const Stop* stop) const {
            return hasher_(stop->coordinates.lat) + hasher_(stop->coordinates.lng) * 47;
        }
    private:
        std::hash<double> hasher_;
    };

    data_base::InputReader* input_reader_ptr_ = nullptr;
    requests::StatReader* stat_reader_ptr_ = nullptr;

    std::deque<Stop> stops_;
    std::deque<Bus> buses_;

    std::unordered_map<std::string_view, Stop*, StringViewHasher> stopname_to_stop_;
    std::unordered_map<std::string_view, Bus*, StringViewHasher> busname_to_bus_;

    std::unordered_map<std::pair<Stop*, Stop*>, int, PairStopsHasher> stops_to_distance_;
    std::unordered_map<Stop*, std::vector<Bus*>, StopHasher> stop_to_buses_;

    void AddStop(const std::string_view name, const geo::Coordinates& coordinates);

    Stop* FindStop(const std::string_view name);

    void AddBus(const std::string_view name, std::vector<Stop*>& route, const char sym);

    Bus* FindBus(const std::string_view name);

    void SetDistanceBetweenStops(const std::string_view first_name, const std::string_view second_name, const int distance);

    int GetDistanceBetweenStops(const std::string_view first_name, const std::string_view second_name);
};

namespace detail {
void DeleteEndSpaces(std::string_view& str);
}

}