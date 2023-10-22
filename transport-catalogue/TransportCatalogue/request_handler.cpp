#include "request_handler.h"

using namespace std;
using namespace request_handler;

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& db, const map_renderer::MapRenderer& renderer)
    : db_(db)
    , renderer_(renderer) {
}

std::optional<RequestHandler::BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    optional<BusStat> dst;
    domain::Bus* bus_ptr = db_.FindBus(bus_name);
    if (bus_ptr == nullptr) {
        return dst;
    }
    int route_length = 0;
    double geo_length = 0.0;
    unordered_set<string_view, domain::StringViewHasher> unique_stops;

    for (size_t counter = 0; counter < bus_ptr->route.size() - 1; ++counter) {
        route_length += db_.GetDistanceBetweenStops(bus_ptr->route.at(counter)->name, bus_ptr->route.at(counter + 1)->name);
        geo_length += geo::ComputeDistance(bus_ptr->route.at(counter)->coordinates, bus_ptr->route.at(counter + 1)->coordinates);
        unique_stops.insert(bus_ptr->route.at(counter)->name);
    }

    unique_stops.insert(bus_ptr->route.at(bus_ptr->route.size() - 1)->name);
    double curvature = double(route_length) / geo_length;

    dst = BusStat{curvature, route_length, static_cast<int>(bus_ptr->route.size()), static_cast<int>(unique_stops.size())};
    return dst;
}

const std::unordered_set<RequestHandler::BusPtr, domain::BusHasher>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    if (db_.FindStop(stop_name) == nullptr) {
        return nullptr;
    }
    return &(db_.FindBusesCrossingStop(db_.FindStop(stop_name)));
}

svg::Document RequestHandler::RenderMap() const {
    const std::deque<domain::Bus>& buses = db_.GetBuses();

    const auto& stop_to_buses = db_.GetStopToBuses();
    std::vector<domain::Stop*> dst;
    for (const auto& stop : db_.GetStops()) {
        if (stop_to_buses.count(const_cast<domain::Stop*>(&stop)) && stop_to_buses.at(const_cast<domain::Stop*>(&stop)).size() != 0) {
            dst.push_back(const_cast<domain::Stop*>(&stop));
        }
    }

    const auto& bus_to_isroundtrip = db_.GetBusToIsroundtrip();

    return renderer_.Render(buses, dst, bus_to_isroundtrip);
}

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */