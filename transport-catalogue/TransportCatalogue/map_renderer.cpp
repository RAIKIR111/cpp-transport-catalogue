#include "map_renderer.h"

using namespace map_renderer;
using namespace std;

bool map_renderer::IsZero(double value) {
    return std::abs(value) < EPSILON;
}

MapRenderer::MapRenderer()
    : settings_(new Settings()) {
}

void MapRenderer::SetWidth(double x) {
    settings_->width = x;
}

void MapRenderer::SetHeight(double x) {
    settings_->height = x;
}

void MapRenderer::SetPadding(double x) {
    settings_->padding = x;
}

void MapRenderer::SetLineWidth(double x) {
    settings_->line_width = x;
}

void MapRenderer::SetStopRadius(double x) {
    settings_->stop_radius = x;
}

void MapRenderer::SetBusLabelFontSize(int x) {
    settings_->bus_label_font_size = x;
}

void MapRenderer::SetBusLabelOffset(const std::vector<double>& x) {
    settings_->bus_label_offset = x;
}

void MapRenderer::SetStopLabelFontSize(int x) {
    settings_->stop_label_font_size = x;
}

void MapRenderer::SetStopLabelOffset(const std::vector<double>& x) {
    settings_->stop_label_offset = x;
}

void MapRenderer::SetUnderlayerColor(svg::Color x) {
    settings_->underlayer_color = x;
}

void MapRenderer::SetUnderlayerWidth(double x) {
    settings_->underlayer_width_ = x;
}

void MapRenderer::SetColorPalette(const std::vector<svg::Color>& x) {
    settings_->color_palette = x;
}

const Settings* MapRenderer::GetSettings() const {
    return settings_;
}

svg::Document MapRenderer::Render(const std::deque<domain::Bus>& buses, const std::vector<domain::Stop*>& active_stops, const std::unordered_map<std::string_view, bool, domain::StringViewHasher>& bus_to_isroundtrip) const {
    const set<const domain::Bus*, CompareBusesByName> buses_in_name_order = GetBusesInNameOrder(buses);

    const vector<geo::Coordinates> bus_stops_coord = GetCoordinatesOfActiveStops(active_stops);

    // У остановок могут быть одинаковые названия

    map_renderer::SphereProjector temp_projector(bus_stops_coord.begin(), bus_stops_coord.end(), GetSettings()->width, GetSettings()->height, GetSettings()->padding);

    svg::Document doc;
    int color_counter = 0;

    vector<svg::Text> text_vec;

    set<pair<string_view, svg::Point>, CompareStopsPairsByName> stop_circles_pair_vec;

    for (const domain::Bus* bus : buses_in_name_order) {
        // constructing route
        const svg::Polyline p = ConstructRouteLines(bus, temp_projector, color_counter, stop_circles_pair_vec);
        doc.Add(move(p));

        // constructing sub text
        const svg::Text sub_text = ConstructBusSubText(bus->name, temp_projector(bus->route.at(0)->coordinates));
        text_vec.push_back(move(sub_text));

        // constructing main text
        const svg::Text main_text = ConstructBusMainText(bus->name, temp_projector(bus->route.at(0)->coordinates), color_counter);
        text_vec.push_back(move(main_text));

        if (!bus_to_isroundtrip.at(bus->name) && (bus->route.at(0) != bus->route.at(bus->route.size() / 2))) {
            const svg::Text sub_text = ConstructBusSubText(bus->name, temp_projector(bus->route.at(static_cast<int>(bus->route.size() / 2))->coordinates));
            text_vec.push_back(move(sub_text));

            const svg::Text main_text = ConstructBusMainText(bus->name, temp_projector(bus->route.at(static_cast<int>(bus->route.size() / 2))->coordinates), color_counter);
            text_vec.push_back(move(main_text));
        }

        ++color_counter;

        if (static_cast<size_t>(color_counter) == GetSettings()->color_palette.size()) {
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
        const svg::Text sub_text = ConstructStopSubText(name, point);
        text_vec.push_back(move(sub_text));

        const svg::Text main_text = ConstructStopMainText(name, point);
        text_vec.push_back(move(main_text));

        const svg::Circle c = ConstructCircle(point);
        doc.Add(move(c));
    }

    for (const auto& text : text_vec) {
        doc.Add(move(text));
    }

    return doc;
}

const svg::Polyline MapRenderer::ConstructRouteLines(const domain::Bus* bus, const map_renderer::SphereProjector& projector, int color_counter, std::set<std::pair<std::string_view, svg::Point>, CompareStopsPairsByName>& stop_circles_pair_vec) const {
    svg::Polyline p;
    p.SetStrokeColor(GetSettings()->color_palette.at(color_counter)).SetFillColor("none"s).SetStrokeWidth(GetSettings()->line_width);
    p.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    for (size_t counter = 0; counter < bus->route.size(); ++counter) {
        // adding point to route
        svg::Point temp_point = projector(bus->route.at(counter)->coordinates);

        p.AddPoint(temp_point);

        stop_circles_pair_vec.insert({bus->route.at(counter)->name, temp_point});
    }

    return p;
}

const svg::Text MapRenderer::ConstructBusSubText(const std::string_view data, const svg::Point& point) const {
    svg::Text sub_text;
    sub_text.SetData(string(data)).SetOffset({GetSettings()->bus_label_offset.at(0), GetSettings()->bus_label_offset.at(1)});
    sub_text.SetFontSize(GetSettings()->bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold"s);
    sub_text.SetPosition(point);

    // additional settings for sub text
    sub_text.SetFillColor(GetSettings()->underlayer_color).SetStrokeColor(GetSettings()->underlayer_color).SetStrokeWidth(GetSettings()->underlayer_width_);
    sub_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    return sub_text;
}

const svg::Text MapRenderer::ConstructBusMainText(const std::string_view data, const svg::Point& point, int color_counter) const {
    svg::Text main_text;
    main_text.SetData(string(data)).SetOffset({GetSettings()->bus_label_offset.at(0), GetSettings()->bus_label_offset.at(1)});
    main_text.SetFontSize(GetSettings()->bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold"s);
    main_text.SetPosition(point);

    // additional settings for main text
    main_text.SetFillColor(GetSettings()->color_palette.at(color_counter));

    return main_text;
}

const svg::Text MapRenderer::ConstructStopSubText(const std::string_view data, const svg::Point& point) const {
    svg::Text sub_text;
    sub_text.SetData(string(data)).SetOffset({GetSettings()->stop_label_offset.at(0), GetSettings()->stop_label_offset.at(1)});
    sub_text.SetFontSize(GetSettings()->stop_label_font_size).SetFontFamily("Verdana"s);
    sub_text.SetPosition(point);

    // additional settings for sub text
    sub_text.SetFillColor(GetSettings()->underlayer_color).SetStrokeColor(GetSettings()->underlayer_color).SetStrokeWidth(GetSettings()->underlayer_width_);
    sub_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    return sub_text;
}

const svg::Text MapRenderer::ConstructStopMainText(const std::string_view data, const svg::Point& point) const {
    svg::Text main_text;
    main_text.SetData(string(data)).SetOffset({GetSettings()->stop_label_offset.at(0), GetSettings()->stop_label_offset.at(1)});
    main_text.SetFontSize(GetSettings()->stop_label_font_size).SetFontFamily("Verdana"s);
    main_text.SetPosition(point);

    // additional settings for main text
    main_text.SetFillColor("black"s);

    return main_text;
}

const svg::Circle MapRenderer::ConstructCircle(const svg::Point& point) const {
    svg::Circle c;
    c.SetCenter(point).SetRadius(GetSettings()->stop_radius).SetFillColor("white"s);
    return c;
}

const std::set<const domain::Bus*, MapRenderer::CompareBusesByName> MapRenderer::GetBusesInNameOrder(const std::deque<domain::Bus>& buses) const {
    set<const domain::Bus*, CompareBusesByName> buses_in_name_order;
    for (const domain::Bus& bus : buses) {
        if (bus.route.size() > 1) {
            buses_in_name_order.insert(&bus);
        }
    }
    return buses_in_name_order;
}

const vector<geo::Coordinates> MapRenderer::GetCoordinatesOfActiveStops(const std::vector<domain::Stop*>& active_stops) const {
    vector<geo::Coordinates> bus_stops_coord;
    bus_stops_coord.reserve(active_stops.size());
    for (const domain::Stop* stop : active_stops) {
        bus_stops_coord.push_back(stop->coordinates);
    }
    return bus_stops_coord;
}

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */