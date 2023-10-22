#pragma once

#include <fstream>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "graph.h"
#include "transport_router.h"
#include "router.h"

class Serialization {
public:
    void SetFilename(const std::string& filename);

    void FullSerialize(const transport_catalogue::TransportCatalogue& catalogue, const map_renderer::MapRenderer& renderer, graph::DirectedWeightedGraph<double>*& graph, const TransportRouter& transport_router) const;

    void FullDeserialize(transport_catalogue::TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer, graph::DirectedWeightedGraph<double>*& graph, graph::Router<double>*& router, TransportRouter& transport_router) const;

private:
    struct CoordinatesHash {
        size_t operator()(const geo::Coordinates& coord) const {
            return std::hash<double>{}(coord.lat) + std::hash<double>{}(coord.lat) * 57;
        }
    };

    void SerializeStops(const transport_catalogue::TransportCatalogue& catalogue, std::ofstream& output) const;
    void SerializeBuses(const transport_catalogue::TransportCatalogue& catalogue, std::ofstream& output) const;
    void SerializeStopsToDistance(const transport_catalogue::TransportCatalogue& catalogue, std::ofstream& output) const;
    void SerializeMapRendererSettings(const map_renderer::MapRenderer& renderer, std::ofstream& output) const;
    void SerializeGraph(graph::DirectedWeightedGraph<double>*& graph, std::ofstream& output) const;
    void SerializeTransportRouter(const TransportRouter& transport_router, std::ofstream& output) const;

    void DeserializeStops(transport_catalogue::TransportCatalogue& catalogue, std::ifstream& input, std::unordered_map<geo::Coordinates, const domain::Stop*, CoordinatesHash>& coords_to_stop) const;
    void DeserializeBuses(transport_catalogue::TransportCatalogue& catalogue, std::ifstream& input, std::unordered_map<geo::Coordinates, const domain::Stop*, CoordinatesHash>& coords_to_stop) const;
    void DeserializeStopsToDistance(transport_catalogue::TransportCatalogue& catalogue, std::ifstream& input, std::unordered_map<geo::Coordinates, const domain::Stop*, CoordinatesHash>& coords_to_stop) const;
    void DeserializeMapRendererSettings(map_renderer::MapRenderer& renderer, std::ifstream& input) const;
    void DeserializeGraph(graph::DirectedWeightedGraph<double>*& graph, std::ifstream& input, graph::Router<double>*& router) const;
    void DeserializeTransportRouter(TransportRouter& transport_router, std::ifstream& input) const;

    static void SerializeColor(std::ofstream& output, const svg::Color& color);

    static svg::Color DeserializeColor(std::ifstream& input);

    template <typename T>
    static void SerializeValue(std::ofstream& output, const T& value);

    static void SerializeString(std::ofstream& output, const std::string& string_to_serialize);

    template <typename T>
    static T DeserializeValue(std::ifstream& input);

    static std::string DeserializeString(std::ifstream& input);

    std::string filename_;
};

template <typename T>
void Serialization::SerializeValue(std::ofstream& output, const T& value) {
    output.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

template <typename T>
T Serialization::DeserializeValue(std::ifstream& input) {
    T value;
    input.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}