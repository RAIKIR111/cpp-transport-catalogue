#include "request_handler.h"

using namespace std;
using namespace request_handler;

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& catalogue_ptr)
    : db_(catalogue_ptr) {
}

std::optional<RequestHandler::BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    optional<BusStat> dst;
    domain::Bus* bus_ptr = db_.FindBus(bus_name);
    if (bus_ptr == nullptr) {
        return dst;
    }
    int route_length = 0;
    double geo_length = 0.0;
    unordered_set<string_view, transport_catalogue::TransportCatalogue::StringViewHasher> unique_stops;

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

const std::unordered_set<RequestHandler::BusPtr, transport_catalogue::TransportCatalogue::BusHasher>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    if (db_.FindStop(stop_name) == nullptr) {
        return nullptr;
    }
    return &(db_.FindBusesCrossingStop(db_.FindStop(stop_name)));
}

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */