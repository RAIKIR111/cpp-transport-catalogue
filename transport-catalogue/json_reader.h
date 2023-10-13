#pragma once

#include <iostream>
#include <cassert>
#include <tuple>
#include <memory>

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace json_reader {

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

class JsonReader {
public:
    explicit JsonReader(std::istream&);

    const json::Document& GetDocument();

    ~JsonReader();

private:
    transport_catalogue::TransportCatalogue catalogue_;
    map_renderer::MapRenderer renderer_;
    TransportRouter transport_router_;
    const request_handler::RequestHandler* req_handler_ = nullptr;
    const json::Document* doc_ = nullptr;

    graph::Router<double>* router_ = nullptr;

    void ProccessBaseRequests(const json::Node& base_requests);

    void ProccessStatRequests(const json::Node& stat_requests, std::ostream& output);

    void ProccessRenderSettings(const json::Node& render_settings);
    
    void ProccessRoutingSettings(const json::Node& routing_settings);

    void ConstructGraph(graph::DirectedWeightedGraph<double>* graph);

    void AddEdgesToGraphCycle(int counter, int till_counter, const domain::Bus& bus, graph::DirectedWeightedGraph<double>* graph);
};

} // namespace json_reader