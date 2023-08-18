#pragma once

#include <iostream>
#include <cassert>
#include <tuple>

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

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
    const request_handler::RequestHandler* req_handler_ = nullptr;
    const json::Document* doc_ = nullptr;

    void ProccessBaseRequests(const json::Node& base_requests);

    void ProccessStatRequests(const json::Node& stat_requests, std::ostream& output);

    void ProccessRenderSettings(const json::Node& render_settings);
};

} // namespace json_reader