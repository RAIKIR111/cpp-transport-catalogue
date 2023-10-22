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

    void FullSerialize(const transport_catalogue::TransportCatalogue& catalogue, const map_renderer::MapRenderer& renderer, graph::DirectedWeightedGraph<double>*& graph, const TransportRouter& transport_router);

    void FullDeserialize(transport_catalogue::TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer, graph::DirectedWeightedGraph<double>*& graph, graph::Router<double>*& router, TransportRouter& transport_router) const;

private:
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