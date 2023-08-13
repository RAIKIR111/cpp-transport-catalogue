#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

using namespace std;
using namespace json_reader;

JsonReader::JsonReader(istream& input)
    : req_handler_(new request_handler::RequestHandler(catalogue_))
    , doc_(new json::Document(json::Load(input)))
    , settings_(new map_renderer::Settings()) {

    assert(doc_->GetRoot().IsMap() == true);

    ProccessBaseRequests(doc_->GetRoot().AsMap().at("base_requests"s));

    ProccessRenderSettings(doc_->GetRoot().AsMap().at("render_settings"s));

    ProccessStatRequests(doc_->GetRoot().AsMap().at("stat_requests"s), cout);

    //RenderAll(cout);
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
            string dst;

            ostringstream oss;
            RenderAll(oss);

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

void JsonReader::ProccessRenderSettings(const json::Node& render_settings) {
    for (const auto& [setting_type, setting] : render_settings.AsMap()) {
        if (setting_type == "width"s) {
            settings_->width = setting.AsDouble();
        }
        else if (setting_type == "height"s) {
            settings_->height = setting.AsDouble();
        }
        else if (setting_type == "padding"s) {
            settings_->padding = setting.AsDouble();
        }
        else if (setting_type == "line_width"s) {
            settings_->line_width = setting.AsDouble();
        }
        else if (setting_type == "stop_radius"s) {
            settings_->stop_radius = setting.AsDouble();
        }
        else if (setting_type == "bus_label_font_size"s) {
            settings_->bus_label_font_size = setting.AsInt();
        }
        else if (setting_type == "bus_label_offset"s) {
            vector<double> offset_vec;
            offset_vec.reserve(setting.AsArray().size());
            for (const auto& item : setting.AsArray()) {
                offset_vec.push_back(item.AsDouble());
            }
            settings_->bus_label_offset = move(offset_vec);
        }
        else if (setting_type == "stop_label_font_size"s) {
            settings_->stop_label_font_size = setting.AsInt();
        }
        else if (setting_type == "stop_label_offset"s) {
            vector<double> offset_vec;
            offset_vec.reserve(setting.AsArray().size());
            for (const auto& item : setting.AsArray()) {
                offset_vec.push_back(item.AsDouble());
            }
            settings_->stop_label_offset = move(offset_vec);
        }
        else if (setting_type == "underlayer_color"s) {
            if (setting.IsString()) {
                settings_->underlayer_color = setting.AsString();
            }
            else if (setting.AsArray().size() == 3) {
                assert(setting.AsArray().at(0).AsInt() >= 0 && setting.AsArray().at(0).AsInt() <= 255);
                assert(setting.AsArray().at(1).AsInt() >= 0 && setting.AsArray().at(1).AsInt() <= 255);
                assert(setting.AsArray().at(2).AsInt() >= 0 && setting.AsArray().at(2).AsInt() <= 255);
                settings_->underlayer_color = svg::Rgb{static_cast<unsigned>(setting.AsArray().at(0).AsInt()), static_cast<unsigned>(setting.AsArray().at(1).AsInt()), static_cast<unsigned>(setting.AsArray().at(2).AsInt())};
            }
            else if (setting.AsArray().size() == 4) {
                assert(setting.AsArray().at(0).AsInt() >= 0 && setting.AsArray().at(0).AsInt() <= 255);
                assert(setting.AsArray().at(1).AsInt() >= 0 && setting.AsArray().at(1).AsInt() <= 255);
                assert(setting.AsArray().at(2).AsInt() >= 0 && setting.AsArray().at(2).AsInt() <= 255);
                assert(setting.AsArray().at(3).AsDouble() >= 0.0 && setting.AsArray().at(3).AsDouble() <= 1.0);
                settings_->underlayer_color = svg::Rgba{static_cast<unsigned>(setting.AsArray().at(0).AsInt()), static_cast<unsigned>(setting.AsArray().at(1).AsInt()), static_cast<unsigned>(setting.AsArray().at(2).AsInt()), setting.AsArray().at(3).AsDouble()};
            }
            else {
                throw std::invalid_argument("wrong color!");
            }
        }
        else if (setting_type == "underlayer_width"s) {
            settings_->underlayer_width_ = setting.AsDouble();
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
            settings_->color_palette = move(colors);
        }
        else {
            throw std::invalid_argument("wrong color!");
        }
    }
}

void JsonReader::RenderAll(std::ostream& output) {
    struct CompareBusesByName {
        bool operator()(const domain::Bus* bus1, const domain::Bus* bus2) const {
            return bus1->name <= bus2->name;
        }
    };

    struct CompareByName {
        bool operator()(const pair<string_view, svg::Point> p1, const pair<string_view, svg::Point> p2) const {
            return p1.first < p2.first;
        }
    };

    set<const domain::Bus*, CompareBusesByName> buses_in_name_order;

    const deque<domain::Bus>& buses = catalogue_.GetBuses();
    for (const domain::Bus& bus : buses) {
        if (bus.route.size() > 1) {
            buses_in_name_order.insert(&bus);
        }
    }

    vector<geo::Coordinates> bus_stops_coord;

    const vector<domain::Stop*>& active_stops = catalogue_.GetActiveStops(); 
    bus_stops_coord.reserve(active_stops.size());
    for (const domain::Stop* stop : active_stops) {
        bus_stops_coord.push_back(stop->coordinates);
    }

    // У остановок могут быть одинаковые названия

    map_renderer::SphereProjector temp_projector(bus_stops_coord.begin(), bus_stops_coord.end(), settings_->width, settings_->height, settings_->padding);

    svg::Document doc;
    int color_counter = 0;

    vector<svg::Text> text_vec;

    set<pair<string_view, svg::Point>, CompareByName> stop_circles_pair_vec;

    for (const domain::Bus* bus : buses_in_name_order) {
        // constructing route
        svg::Polyline p;
        p.SetStrokeColor(settings_->color_palette.at(color_counter)).SetFillColor("none"s).SetStrokeWidth(settings_->line_width);
        p.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (size_t counter = 0; counter < bus->route.size(); ++counter) {
            // adding point to route
            svg::Point temp_point = temp_projector(bus->route.at(counter)->coordinates);

            p.AddPoint(temp_point);

            stop_circles_pair_vec.insert({bus->route.at(counter)->name, temp_point});
        }

        // adding route
        doc.Add(move(p));

        // constructing text
        svg::Text sub_text;
        sub_text.SetData(string(bus->name)).SetOffset({settings_->bus_label_offset.at(0), settings_->bus_label_offset.at(1)});
        sub_text.SetFontSize(settings_->bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold"s);
        sub_text.SetPosition(temp_projector(bus->route.at(0)->coordinates));

        // additional settings for sub text
        sub_text.SetFillColor(settings_->underlayer_color).SetStrokeColor(settings_->underlayer_color).SetStrokeWidth(settings_->underlayer_width_);
        sub_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        text_vec.push_back(move(sub_text));

        svg::Text main_text;
        main_text.SetData(string(bus->name)).SetOffset({settings_->bus_label_offset.at(0), settings_->bus_label_offset.at(1)});
        main_text.SetFontSize(settings_->bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold"s);
        main_text.SetPosition(temp_projector(bus->route.at(0)->coordinates));

        // additional settings for main text
        main_text.SetFillColor(settings_->color_palette.at(color_counter));

        text_vec.push_back(move(main_text));  // 1 2 3 4 5 4 3 2 1     1 2 3 4 3 2 1   0 1 2 3 2 1 0

        if (!catalogue_.IsRoundTrip(bus->name) && (bus->route.at(0) != bus->route.at(bus->route.size() / 2))) {
            svg::Text sub_text;
            sub_text.SetData(string(bus->name)).SetOffset({settings_->bus_label_offset.at(0), settings_->bus_label_offset.at(1)});
            sub_text.SetFontSize(settings_->bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold"s);
            sub_text.SetPosition(temp_projector(bus->route.at(bus->route.size() / 2)->coordinates));

            // additional settings for sub text
            sub_text.SetFillColor(settings_->underlayer_color).SetStrokeColor(settings_->underlayer_color).SetStrokeWidth(settings_->underlayer_width_);
            sub_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            text_vec.push_back(move(sub_text));

            svg::Text main_text;
            main_text.SetData(string(bus->name)).SetOffset({settings_->bus_label_offset.at(0), settings_->bus_label_offset.at(1)});
            main_text.SetFontSize(settings_->bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold"s);
            main_text.SetPosition(temp_projector(bus->route.at(bus->route.size() / 2)->coordinates));

            // additional settings for main text
            main_text.SetFillColor(settings_->color_palette.at(color_counter));

            text_vec.push_back(move(main_text));
        }

        ++color_counter;

        if (static_cast<size_t>(color_counter) == settings_->color_palette.size()) {
            color_counter = 0;
        }
    }

    // adding texts
    for (const auto& text : text_vec) {
        doc.Add(move(text));
    }

    text_vec.clear();
    text_vec.reserve(stop_circles_pair_vec.size());

    // adding stop circles
    for (const auto& [name, point] : stop_circles_pair_vec) {
        // constructing text
        svg::Text sub_text;
        sub_text.SetData(string(name)).SetOffset({settings_->stop_label_offset.at(0), settings_->stop_label_offset.at(1)});
        sub_text.SetFontSize(settings_->stop_label_font_size).SetFontFamily("Verdana"s);
        sub_text.SetPosition(point);

        // additional settings for sub text
        sub_text.SetFillColor(settings_->underlayer_color).SetStrokeColor(settings_->underlayer_color).SetStrokeWidth(settings_->underlayer_width_);
        sub_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        text_vec.push_back(move(sub_text));

        svg::Text main_text;
        main_text.SetData(string(name)).SetOffset({settings_->stop_label_offset.at(0), settings_->stop_label_offset.at(1)});
        main_text.SetFontSize(settings_->stop_label_font_size).SetFontFamily("Verdana"s);
        main_text.SetPosition(point);

        // additional settings for main text
        main_text.SetFillColor("black"s);

        text_vec.push_back(move(main_text));

        svg::Circle c;
        c.SetCenter(point).SetRadius(settings_->stop_radius).SetFillColor("white"s);
        doc.Add(move(c));
    }

    for (const auto& text : text_vec) {
        doc.Add(move(text));
    }

    doc.Render(output);
}

JsonReader::~JsonReader() {
    if (doc_) {
        delete doc_;
    }
    if (req_handler_) {
        delete req_handler_;
    }
}