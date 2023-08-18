#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

using namespace std;
using namespace json_reader;

JsonReader::JsonReader(istream& input)
    : req_handler_(new request_handler::RequestHandler(catalogue_, renderer_))
    , doc_(new json::Document(json::Load(input))) {

    assert(doc_->GetRoot().IsMap() == true);

    ProccessBaseRequests(doc_->GetRoot().AsMap().at("base_requests"s));

    ProccessRenderSettings(doc_->GetRoot().AsMap().at("render_settings"s));

    ProccessStatRequests(doc_->GetRoot().AsMap().at("stat_requests"s), cout);
}

void JsonReader::ProccessBaseRequests(const json::Node& base_requests) {
    assert(base_requests.IsArray() == true);

    // filling transport catalogue
    vector<tuple<string_view, vector<string_view>, bool>> buses_requests; // bus name, stops, is roundtrip?
    for (const json::Node& node_request : base_requests.AsArray()) {
        if (node_request.IsMap() == false) {
            return;
        }
        const json::Dict& temp_dict = node_request.AsMap();
        if (temp_dict.at("type"s) == "Stop"s) {
            catalogue_.AddStop(temp_dict.at("name"s).AsString(),
                geo::Coordinates{temp_dict.at("latitude"s).AsDouble(), temp_dict.at("longitude"s).AsDouble()});

            for (const auto& [another_stop_name, dis] : temp_dict.at("road_distances"s).AsMap()) {
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
    for (const auto& [setting_type, setting] : render_settings.AsMap()) {
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

void JsonReader::ProccessStatRequests(const json::Node& stat_requests, ostream& output) {
    json::Array dst_array;
    for (const auto& request : stat_requests.AsArray()) {
        if (request.IsMap() == false) {
            return;
        }
        const json::Dict& current_dict = request.AsMap();
        int id = 0;
        string_view type;
        string_view name;
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
        }
        if (type == "Stop"sv) {
            if (req_handler_->GetBusesByStop(name) == nullptr) {
                json::Dict temp_dict{{"error_message"s, "not found"s}, {"request_id"s, id}};
                dst_array.push_back(temp_dict);
            }
            else {
                json::Array buses;
                const auto& buses_crossing_stop = req_handler_->GetBusesByStop(name);

                if (buses_crossing_stop != nullptr) {
                    for (const auto& bus_ptr : (*buses_crossing_stop)) {
                        buses.push_back(string(bus_ptr->name));
                    }     
                }
                sort(buses.begin(), buses.end(), [](const json::Node& lhs, const json::Node& rhs) {
                    return (lhs.AsString() < rhs.AsString());
                });

                json::Dict temp_dict{{"buses"s, buses}, {"request_id"s, id}};

                dst_array.push_back(temp_dict);
            }
        }
        else if (type == "Bus"sv) {
            const auto& bus_stat = req_handler_->GetBusStat(name);
            if (!bus_stat.has_value()) {
                json::Dict temp_dict{{"error_message"s, "not found"s}, {"request_id"s, id}};
                dst_array.push_back(temp_dict);
            }
            else {
                json::Dict temp_dict{{"curvature"s, bus_stat.value().curvature}, {"request_id"s, id},
                    {"route_length"s, bus_stat.value().route_length}, {"stop_count"s, bus_stat.value().stop_count},
                    {"unique_stop_count"s, bus_stat.value().unique_stop_count}};

                dst_array.push_back(temp_dict);
            }
        }
        else if (type == "Map"sv) {
            // Передаем автобусы
            // Ищем активные остановки и передаем их
            // Передаем bus_to_isroundtrip_
            string dst;
            ostringstream oss;
            
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

            json::Dict temp_dict{{"map"s, dst}, {"request_id"s, id}};

            dst_array.push_back(temp_dict);
        }
    }

    json::Print(json::Document(json::Node(dst_array)), output);
}

JsonReader::~JsonReader() {
    if (doc_) {
        delete doc_;
    }
    if (req_handler_) {
        delete req_handler_;
    }
}