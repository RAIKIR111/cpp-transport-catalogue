#pragma once

#include "string_view"
#include "vector"

#include "geo.h"

namespace domain {

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

struct Stop {
    std::string_view name;
    geo::Coordinates coordinates;
};

struct Bus {
    std::string_view name;
    std::vector<Stop*> route;
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

} // namespace domain