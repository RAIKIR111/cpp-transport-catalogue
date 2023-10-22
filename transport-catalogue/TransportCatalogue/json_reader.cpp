#include "json_reader.h"
#include "json_builder.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

using namespace std;
using namespace json_reader;

JsonReader::JsonReader(istream& input)
    : req_handler_(new request_handler::RequestHandler(catalogue_, renderer_))
    , doc_(new json::Document(json::Load(input))) {

    assert(doc_->GetRoot().IsDict() == true);
}

void JsonReader::MakeBase() {
    ProccessBaseRequests(doc_->GetRoot().AsDict().at("base_requests"s));

    ProccessRenderSettings(doc_->GetRoot().AsDict().at("render_settings"s));

    ProccessRoutingSettings(doc_->GetRoot().AsDict().at("routing_settings"s));

    ProccessSerializationSettings(doc_->GetRoot().AsDict().at("serialization_settings"s));

    graph_ = new graph::DirectedWeightedGraph<double>(catalogue_.GetStops().size() * 2);
    ConstructGraph(graph_);

    //router_ = new graph::Router(*graph);!!!!!!!!!!!!!!!!!!!!!!!!!!

    serialization_obj_.FullSerialize(catalogue_, renderer_, graph_, transport_router_);
}

void JsonReader::ProccessRequests() {
    ProccessSerializationSettings(doc_->GetRoot().AsDict().at("serialization_settings"s));

    serialization_obj_.FullDeserialize(catalogue_, renderer_, graph_, router_, transport_router_);

    ProccessStatRequests(doc_->GetRoot().AsDict().at("stat_requests"s), cout);
}

void JsonReader::ProccessBaseRequests(const json::Node& base_requests) {
    assert(base_requests.IsArray() == true);

    // filling transport catalogue
    vector<tuple<string_view, vector<string_view>, bool>> buses_requests; // bus name, stops, is roundtrip?
    for (const json::Node& node_request : base_requests.AsArray()) {
        if (node_request.IsDict() == false) {
            return;
        }
        const json::Dict& temp_dict = node_request.AsDict();
        if (temp_dict.at("type"s) == "Stop"s) {
            catalogue_.AddStop(temp_dict.at("name"s).AsString(),
                geo::Coordinates{temp_dict.at("latitude"s).AsDouble(), temp_dict.at("longitude"s).AsDouble()});

            for (const auto& [another_stop_name, dis] : temp_dict.at("road_distances"s).AsDict()) {
                catalogue_.SetDistanceBetweenStops(temp_dict.at("name"s).AsString(), another_stop_name, dis.AsInt());
            }
        }
        else if (temp_dict.at("type"s) == "Bus"s) {
            vector<string_view> temp_stops;
            for (const auto& item : temp_dict.at("stops"s).AsArray()) {
                const string& temp_stop_name = item.AsString();
                temp_stops.push_back(string_view(temp_stop_name));
            }
            buses_requests.push_back(make_tuple(string_view(temp_dict.at("name"s).AsString()), temp_stops, temp_dict.at("is_roundtrip"s).AsBool()));
        }
    }
    for (const auto& item : buses_requests) {
        catalogue_.AddBus(get<0>(item), get<1>(item), get<2>(item));
    }
}

void JsonReader::ProccessRenderSettings(const json::Node& render_settings) {
    for (const auto& [setting_type, setting] : render_settings.AsDict()) {
        if (setting_type == "width"s) {
            renderer_.SetWidth(setting.AsDouble());
        }
        else if (setting_type == "height"s) {
            renderer_.SetHeight(setting.AsDouble());
        }
        else if (setting_type == "padding"s) {
            renderer_.SetPadding(setting.AsDouble());
        }
        else if (setting_type == "line_width"s) {
            renderer_.SetLineWidth(setting.AsDouble());
        }
        else if (setting_type == "stop_radius"s) {
            renderer_.SetStopRadius(setting.AsDouble());
        }
        else if (setting_type == "bus_label_font_size"s) {
            renderer_.SetBusLabelFontSize(setting.AsInt());
        }
        else if (setting_type == "bus_label_offset"s) {
            vector<double> offset_vec;
            offset_vec.reserve(setting.AsArray().size());
            for (const auto& item : setting.AsArray()) {
                offset_vec.push_back(item.AsDouble());
            }
            renderer_.SetBusLabelOffset(move(offset_vec));
        }
        else if (setting_type == "stop_label_font_size"s) {
            renderer_.SetStopLabelFontSize(setting.AsInt());
        }
        else if (setting_type == "stop_label_offset"s) {
            vector<double> offset_vec;
            offset_vec.reserve(setting.AsArray().size());
            for (const auto& item : setting.AsArray()) {
                offset_vec.push_back(item.AsDouble());
            }
            renderer_.SetStopLabelOffset(move(offset_vec));
        }
        else if (setting_type == "underlayer_color"s) {
            if (setting.IsString()) {
                renderer_.SetUnderlayerColor(setting.AsString());
            }
            else if (setting.AsArray().size() == 3) {
                assert(setting.AsArray().at(0).AsInt() >= 0 && setting.AsArray().at(0).AsInt() <= 255);
                assert(setting.AsArray().at(1).AsInt() >= 0 && setting.AsArray().at(1).AsInt() <= 255);
                assert(setting.AsArray().at(2).AsInt() >= 0 && setting.AsArray().at(2).AsInt() <= 255);
                svg::Color color = svg::Rgb{static_cast<unsigned>(setting.AsArray().at(0).AsInt()), static_cast<unsigned>(setting.AsArray().at(1).AsInt()), static_cast<unsigned>(setting.AsArray().at(2).AsInt())};
                renderer_.SetUnderlayerColor(move(color));
            }
            else if (setting.AsArray().size() == 4) {
                assert(setting.AsArray().at(0).AsInt() >= 0 && setting.AsArray().at(0).AsInt() <= 255);
                assert(setting.AsArray().at(1).AsInt() >= 0 && setting.AsArray().at(1).AsInt() <= 255);
                assert(setting.AsArray().at(2).AsInt() >= 0 && setting.AsArray().at(2).AsInt() <= 255);
                assert(setting.AsArray().at(3).AsDouble() >= 0.0 && setting.AsArray().at(3).AsDouble() <= 1.0);
                svg::Color color = svg::Rgba{static_cast<unsigned>(setting.AsArray().at(0).AsInt()), static_cast<unsigned>(setting.AsArray().at(1).AsInt()), static_cast<unsigned>(setting.AsArray().at(2).AsInt()), setting.AsArray().at(3).AsDouble()};
                renderer_.SetUnderlayerColor(move(color));
            }
            else {
                throw std::invalid_argument("wrong color!");
            }
        }
        else if (setting_type == "underlayer_width"s) {
            renderer_.SetUnderlayerWidth(setting.AsDouble());
        }
        else if (setting_type == "color_palette"s) {
            vector<svg::Color> colors;
            colors.reserve(setting.AsArray().size());
            for (const auto& item : setting.AsArray()) {
                if (item.IsString()) {
                    colors.push_back(item.AsString());
                }
                else if (item.AsArray().size() == 3) {
                    assert(item.AsArray().at(0).AsInt() >= 0 && item.AsArray().at(0).AsInt() <= 255);
                    assert(item.AsArray().at(1).AsInt() >= 0 && item.AsArray().at(1).AsInt() <= 255);
                    assert(item.AsArray().at(2).AsInt() >= 0 && item.AsArray().at(2).AsInt() <= 255);
                    colors.push_back(svg::Rgb{static_cast<unsigned>(item.AsArray().at(0).AsInt()), static_cast<unsigned>(item.AsArray().at(1).AsInt()), static_cast<unsigned>(item.AsArray().at(2).AsInt())});
                }
                else if (item.AsArray().size() == 4) {
                    assert(item.AsArray().at(0).AsInt() >= 0 && item.AsArray().at(0).AsInt() <= 255);
                    assert(item.AsArray().at(1).AsInt() >= 0 && item.AsArray().at(1).AsInt() <= 255);
                    assert(item.AsArray().at(2).AsInt() >= 0 && item.AsArray().at(2).AsInt() <= 255);
                    assert(item.AsArray().at(3).AsDouble() >= 0.0 && item.AsArray().at(3).AsDouble() <= 1.0);
                    colors.push_back(svg::Rgba{static_cast<unsigned>(item.AsArray().at(0).AsInt()), static_cast<unsigned>(item.AsArray().at(1).AsInt()), static_cast<unsigned>(item.AsArray().at(2).AsInt()), item.AsArray().at(3).AsDouble()});
                }
                else {
                    throw std::invalid_argument("wrong color!");
                }
            }
            renderer_.SetColorPalette(move(colors));
        }
        else {
            throw std::invalid_argument("wrong color!");
        }
    }
}

void JsonReader::ProccessRoutingSettings(const json::Node& routing_settings) {
    for (const auto& [setting_type, setting] : routing_settings.AsDict()) {
        if (setting_type == "bus_wait_time"s) {
            transport_router_.SetBusWaitTime(setting.AsInt());
        }
        else if (setting_type == "bus_velocity"s) {
            transport_router_.SetBusVelocity(setting.AsDouble());
        }
    }
}

void JsonReader::ProccessSerializationSettings(const json::Node& serialization_settings) {
    for (const auto& [setting_type, setting] : serialization_settings.AsDict()) {
        if (setting_type == "file"s) {
            serialization_obj_.SetFilename(setting.AsString());
        }
    }
}

void JsonReader::ProccessStatRequests(const json::Node& stat_requests, ostream& output) {
    json::Builder builder{};
    builder.StartArray();

    for (const auto& request : stat_requests.AsArray()) {
        if (request.IsDict() == false) {
            return;
        }
        const json::Dict& current_dict = request.AsDict();
        int id = 0;
        string_view type;
        string_view name;
        string_view from;
        string_view to;
        for (const auto& item : current_dict) {
            if (item.first == "id"s) {
                id = item.second.AsInt();
            }
            else if (item.first == "type"s) {
                type = item.second.AsString();
            }
            else if (item.first == "name"s) {
                name = item.second.AsString();
            }
            else if (item.first == "from"s) {
                from = item.second.AsString();
            }
            else if (item.first == "to"s) {
                to = item.second.AsString();
            }
        }
        if (type == "Stop"sv) {
            if (req_handler_->GetBusesByStop(name) == nullptr) {
                builder.StartDict().Key("error_message"s).Value("not found"s).Key("request_id"s).Value(id).EndDict();
            }
            else {
                vector<string> buses;
                const auto& buses_crossing_stop = req_handler_->GetBusesByStop(name);

                if (buses_crossing_stop != nullptr) {
                    for (const auto& bus_ptr : (*buses_crossing_stop)) {
                        buses.push_back(string(bus_ptr->name));
                    }     
                }

                sort(buses.begin(), buses.end());

                builder.StartDict().Key("buses"s).StartArray();
                for (const auto& item : buses) {
                    builder.Value(item);
                }
                builder.EndArray().Key("request_id"s).Value(id).EndDict();
            }
        }
        else if (type == "Bus"sv) {
            const auto& bus_stat = req_handler_->GetBusStat(name);
            if (!bus_stat.has_value()) {
                builder.StartDict().Key("error_message"s).Value("not found"s).Key("request_id"s).Value(id).EndDict();
            }
            else {
                builder.StartDict().Key("curvature"s).Value(bus_stat.value().curvature).Key("request_id"s).Value(id)
                .Key("route_length"s).Value(bus_stat.value().route_length).Key("stop_count"s).Value(bus_stat.value().stop_count)
                .Key("unique_stop_count"s).Value(bus_stat.value().unique_stop_count).EndDict();
            }
        }
        else if (type == "Map"sv) {
            string dst;
            ostringstream oss;
            
            if (req_handler_ == nullptr) {
                
            }
            const svg::Document doc_dst = req_handler_->RenderMap();
            doc_dst.Render(oss);

            istringstream input_stream(oss.str());

            string line;
            if (getline(input_stream, line) && line.size() != 0) {
                dst += line;
            }
            while (getline(input_stream, line) && line.size() != 0) {
                dst += '\n';
                dst += line;
            }

            builder.StartDict().Key("map"s).Value(dst).Key("request_id"s).Value(id).EndDict();
        }
        else if (type == "Route"sv) {
            /*if (router_ == nullptr) {
                graph::DirectedWeightedGraph<double>* graph = new graph::DirectedWeightedGraph<double>(catalogue_.GetStops().size() * 2);
                ConstructGraph(graph);

                router_ = new graph::Router(*graph);
            }*/
            int vertex_1 = transport_router_.GetVertexByStopname(from);
            int vertex_2 = transport_router_.GetVertexByStopname(to);
            auto answer = router_->BuildRoute(vertex_1 * 2, vertex_2 * 2);
            if (!answer.has_value()) {
                builder.StartDict().Key("error_message"s).Value("not found"s).Key("request_id"s).Value(id).EndDict();
            }
            else {
                builder.StartDict().Key("items"s).StartArray();
                const std::vector<graph::EdgeId>& route_edges = answer.value().edges;
                for (auto counter = 0; counter < route_edges.size(); ++counter) {
                    const graph::Edge<double>& temp_edge = router_->GetGraph().GetEdge(route_edges.at(counter));
                    const std::pair<std::string_view, int>& temp_edge_data = transport_router_.GetEdgeData(temp_edge);
                    if (temp_edge_data.second == 0) {
                        builder.StartDict().Key("stop_name"s).Value(string(temp_edge_data.first)).Key("time"s).Value(temp_edge.weight);
                        builder.Key("type"s).Value("Wait").EndDict();
                    }
                    else {
                        builder.StartDict().Key("bus"s).Value(string(temp_edge_data.first)).Key("span_count"s).Value(temp_edge_data.second);
                        builder.Key("time"s).Value(temp_edge.weight).Key("type"s).Value("Bus"s).EndDict();
                    }
                }
                builder.EndArray();
                builder.Key("request_id"s).Value(id).Key("total_time"s).Value(answer.value().weight);
                builder.EndDict();
            }

        }
    }

    builder.EndArray();

    json::Print(json::Document(builder.Build()), output);
}

void JsonReader::ConstructGraph(graph::DirectedWeightedGraph<double>* graph) {
    const std::deque<domain::Stop>& stops = catalogue_.GetStops();
    const auto& wait_time = transport_router_.GetBusWaitTime();
    for (auto counter = 0; counter < stops.size(); ++counter) {
        graph::Edge<double> temp_edge(static_cast<graph::VertexId>(counter * 2), static_cast<graph::VertexId>(counter * 2 + 1), double(wait_time));
        graph->AddEdge(temp_edge);
        transport_router_.UpdStopnameToVertex(stops.at(counter).name, counter);
        transport_router_.UpdEdgesData(temp_edge, stops.at(counter).name, 0);
    }

    for (const auto& bus : catalogue_.GetBuses()) {
        if (catalogue_.GetBusToIsroundtrip().at(bus.name) == true) {
            AddEdgesToGraphCycle(0, bus.route.size() - 1, bus, graph);
        }
        else {
            AddEdgesToGraphCycle(0, bus.route.size() / 2 + 1, bus, graph);
            AddEdgesToGraphCycle(bus.route.size() / 2, bus.route.size(), bus, graph);
        }
    }
}

void JsonReader::AddEdgesToGraphCycle(int counter, int till_counter, const domain::Bus& bus, graph::DirectedWeightedGraph<double>* graph) {
    for (auto main_counter = counter; main_counter < till_counter; ++main_counter) {
        int main_vertex = transport_router_.GetVertexByStopname(bus.route.at(main_counter)->name);
        double dist = 0;
        for (auto counter = main_counter + 1; counter < bus.route.size(); ++counter) {
            dist += catalogue_.GetDistanceBetweenStops(bus.route.at(counter - 1)->name, bus.route.at(counter)->name);
            int temp_vertex = transport_router_.GetVertexByStopname(bus.route.at(counter)->name);
            const double travel_time = double(dist) / (transport_router_.GetBusVelocity() * 1000.0 / 60.0);
            graph::Edge<double> temp_edge(static_cast<graph::VertexId>(main_vertex * 2 + 1), static_cast<graph::VertexId>(temp_vertex * 2), travel_time);
            graph->AddEdge(temp_edge);
            transport_router_.UpdEdgesData(temp_edge, bus.name, counter - main_counter);
        }
    }
}

JsonReader::~JsonReader() {
    if (doc_) {
        delete doc_;
    }
    if (req_handler_) {
        delete req_handler_;
    }
    if (router_) {
        delete router_;
    }
}