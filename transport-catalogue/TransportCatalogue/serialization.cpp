#include "serialization.h"

using namespace std;

void Serialization::SetFilename(const std::string& filename) {
    filename_ = filename;
}

void Serialization::FullSerialize(const transport_catalogue::TransportCatalogue& catalogue, const map_renderer::MapRenderer& renderer, graph::DirectedWeightedGraph<double>*& graph, const TransportRouter& transport_router) const {
    std::ofstream output(filename_, std::ios::binary);
    SerializeStops(catalogue, output);
    SerializeBuses(catalogue, output);
    SerializeStopsToDistance(catalogue, output);
    SerializeMapRendererSettings(renderer, output);
    SerializeGraph(graph, output);
    SerializeTransportRouter(transport_router, output);
}

void Serialization::FullDeserialize(transport_catalogue::TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer, graph::DirectedWeightedGraph<double>*& graph, graph::Router<double>*& router, TransportRouter& transport_router) const {
    std::ifstream input(filename_, std::ios::binary);
    std::unordered_map<geo::Coordinates, const domain::Stop*, CoordinatesHash> coords_to_stop;
    DeserializeStops(catalogue, input, coords_to_stop);
    DeserializeBuses(catalogue, input, coords_to_stop);
    DeserializeStopsToDistance(catalogue, input, coords_to_stop);
    DeserializeMapRendererSettings(renderer, input);
    DeserializeGraph(graph, input, router);
    DeserializeTransportRouter(transport_router, input);
}

//////////////////////////////////////////////////////////

void Serialization::SerializeStops(const transport_catalogue::TransportCatalogue& catalogue, std::ofstream& output) const {
    SerializeValue(output, catalogue.GetStops().size());
    for (const domain::Stop& stop : catalogue.GetStops()) {
        SerializeString(output, stop.name);
        SerializeValue(output, stop.coordinates);
    }
}

void Serialization::SerializeBuses(const transport_catalogue::TransportCatalogue& catalogue, std::ofstream& output) const {
    SerializeValue(output, catalogue.GetBuses().size());
    for (const domain::Bus& bus : catalogue.GetBuses()) {
        SerializeValue(output, catalogue.GetBusToIsroundtrip().at(bus.name));
        SerializeString(output, bus.name);
        SerializeValue(output, bus.route.size());
        for (const auto& stop : bus.route) {
            SerializeValue(output, stop->coordinates);
        }
    }
}

void Serialization::SerializeStopsToDistance(const transport_catalogue::TransportCatalogue& catalogue, std::ofstream& output) const {
    SerializeValue(output, catalogue.GetStopsToDistance().size());
    for (const auto& [stops_pair, distance] : catalogue.GetStopsToDistance()) {
        SerializeValue(output, stops_pair.first->coordinates);
        SerializeValue(output, stops_pair.second->coordinates);
        SerializeValue(output, distance);
    }
}

void Serialization::SerializeMapRendererSettings(const map_renderer::MapRenderer& renderer, std::ofstream& output) const {
    const map_renderer::Settings* settings = renderer.GetSettings();
    SerializeValue(output, settings->width);
    SerializeValue(output, settings->height);
    SerializeValue(output, settings->padding);
    SerializeValue(output, settings->line_width);
    SerializeValue(output, settings->stop_radius);
    SerializeValue(output, settings->bus_label_font_size);

    SerializeValue(output, settings->bus_label_offset.size());
    for (const auto& double_value : settings->bus_label_offset) {
        SerializeValue(output, double_value);
    }

    SerializeValue(output, settings->stop_label_font_size);

    SerializeValue(output, settings->stop_label_offset.size());
    for (const auto& double_value : settings->stop_label_offset) {
        SerializeValue(output, double_value);
    }

    SerializeColor(output, settings->underlayer_color);
    SerializeValue(output, settings->underlayer_width_);

    SerializeValue(output, settings->color_palette.size());
    for (const auto& color : settings->color_palette) {
        SerializeColor(output, color);
    }
}

void Serialization::SerializeGraph(graph::DirectedWeightedGraph<double>*& graph, std::ofstream& output) const {
    SerializeValue(output, graph->GetEdgeCount());
    for (auto counter = 0; counter < graph->GetEdgeCount(); ++counter) {
        SerializeValue(output, graph->GetEdge(counter));
    }
    SerializeValue(output, graph->GetFullIncidenceLists().size());
    for (auto counter = 0; counter < graph->GetFullIncidenceLists().size(); ++counter) {
        const std::vector<graph::EdgeId>& temp_v = graph->GetFullIncidenceLists().at(counter);
        SerializeValue(output, temp_v.size());
        for (auto in_counter = 0; in_counter < temp_v.size(); ++in_counter) {
            SerializeValue(output, temp_v.at(in_counter));
        }
    }
}

void Serialization::SerializeTransportRouter(const TransportRouter& transport_router, std::ofstream& output) const {
    SerializeValue(output, transport_router.GetBusWaitTime());
    SerializeValue(output, transport_router.GetBusVelocity());

    SerializeValue(output, transport_router.GetFullStopnameToVertex().size());
    for (const auto& item : transport_router.GetFullStopnameToVertex()) {
        SerializeString(output, item.first);
        SerializeValue(output, item.second);
    }

    SerializeValue(output, transport_router.GetFullEdgeToData().size());
    for (const auto& item : transport_router.GetFullEdgeToData()) {
        const graph::Edge<double>& temp_edge = item.first;
        SerializeValue(output, temp_edge);
        SerializeString(output, item.second.first);
        SerializeValue(output, item.second.second);
    }
}

void Serialization::DeserializeStops(transport_catalogue::TransportCatalogue& catalogue, std::ifstream& input, std::unordered_map<geo::Coordinates, const domain::Stop*, CoordinatesHash>& coords_to_stop) const {
    size_t length = DeserializeValue<size_t>(input);
    for (auto counter = 0; counter < length; ++counter) {
        std::string name = DeserializeString(input);

        geo::Coordinates temp_coords = DeserializeValue<geo::Coordinates>(input);

        catalogue.AddStop(name, temp_coords);
        coords_to_stop[temp_coords] = &catalogue.GetStops().back();
    }
}

void Serialization::DeserializeBuses(transport_catalogue::TransportCatalogue& catalogue, std::ifstream& input, std::unordered_map<geo::Coordinates, const domain::Stop*, CoordinatesHash>& coords_to_stop) const {
    size_t length = DeserializeValue<size_t>(input);
    for (auto counter = 0; counter < length; ++counter) {
        std::vector<std::string_view> entry_route;

        bool is_roundtrip = DeserializeValue<bool>(input);

        std::string name = DeserializeString(input);

        size_t route_length = DeserializeValue<size_t>(input);
        for (auto inner_counter = 0; inner_counter < route_length; ++inner_counter) {
            geo::Coordinates temp_coords = DeserializeValue<geo::Coordinates>(input);

            entry_route.push_back(coords_to_stop.at(temp_coords)->name);
        }

        catalogue.AddBus(name, entry_route, is_roundtrip, true);
    }
}

void Serialization::DeserializeStopsToDistance(transport_catalogue::TransportCatalogue& catalogue, std::ifstream& input, std::unordered_map<geo::Coordinates, const domain::Stop*, CoordinatesHash>& coords_to_stop) const {
    size_t length = DeserializeValue<size_t>(input);
    for (auto counter = 0; counter < length; ++counter) {
        geo::Coordinates temp_coords1 = DeserializeValue<geo::Coordinates>(input);
        geo::Coordinates temp_coords2 = DeserializeValue<geo::Coordinates>(input);
        int distance = DeserializeValue<int>(input);
        catalogue.SetDistanceBetweenStops(coords_to_stop.at(temp_coords1)->name, coords_to_stop.at(temp_coords2)->name, distance);
    }
}

void Serialization::DeserializeMapRendererSettings(map_renderer::MapRenderer& renderer, std::ifstream& input) const {
    renderer.SetWidth(DeserializeValue<double>(input));
    renderer.SetHeight(DeserializeValue<double>(input));
    renderer.SetPadding(DeserializeValue<double>(input));
    renderer.SetLineWidth(DeserializeValue<double>(input));
    renderer.SetStopRadius(DeserializeValue<double>(input));

    renderer.SetBusLabelFontSize(DeserializeValue<int>(input));

    std::vector<double> temp_v(DeserializeValue<size_t>(input));
    for (double& item : temp_v) {
        item = DeserializeValue<double>(input);
    }
    renderer.SetBusLabelOffset(temp_v);

    renderer.SetStopLabelFontSize(DeserializeValue<int>(input));

    std::vector<double> temp_v1(DeserializeValue<size_t>(input));
    for (auto& item : temp_v1) {
        item = DeserializeValue<double>(input);
    }
    renderer.SetStopLabelOffset(temp_v1);

    renderer.SetUnderlayerColor(DeserializeColor(input));

    renderer.SetUnderlayerWidth(DeserializeValue<double>(input));

    std::vector<svg::Color> temp_v2(DeserializeValue<size_t>(input));
    for (auto& item : temp_v2) {
        item = DeserializeColor(input);
    }
    renderer.SetColorPalette(temp_v2);
}

void Serialization::DeserializeGraph(graph::DirectedWeightedGraph<double>*& graph, std::ifstream& input, graph::Router<double>*& router) const {
    std::vector<graph::Edge<double>> edges(DeserializeValue<size_t>(input));
    for (auto& item : edges) {
        item = DeserializeValue<graph::Edge<double>>(input);
    }

    std::vector<std::vector<graph::EdgeId>> incidence_lists(DeserializeValue<size_t>(input));
    for (std::vector<graph::EdgeId>& item : incidence_lists) {
        item.resize(DeserializeValue<size_t>(input));
        for (auto& in_item : item) {
            in_item = DeserializeValue<size_t>(input);
        }
    }
    
    graph = new graph::DirectedWeightedGraph<double>{edges, incidence_lists};
    router = new graph::Router<double>(*graph);
}

void Serialization::DeserializeTransportRouter(TransportRouter& transport_router, std::ifstream& input) const {
    transport_router.SetBusWaitTime(DeserializeValue<int>(input));
    transport_router.SetBusVelocity(DeserializeValue<double>(input));

    size_t length = DeserializeValue<size_t>(input);
    for (auto counter = 0; counter < length; ++counter) {
        auto temp_str = DeserializeString(input);
        auto temp_val = DeserializeValue<int>(input);
        transport_router.UpdStopnameToVertex(temp_str, temp_val);
    }

    length = DeserializeValue<size_t>(input);
    for (auto counter = 0; counter < length; ++counter) {
        graph::Edge<double> temp_edge = DeserializeValue<graph::Edge<double>>(input);
        auto temp_str = DeserializeString(input);
        auto temp_val = DeserializeValue<int>(input);
        transport_router.UpdEdgesData(temp_edge, temp_str, temp_val);
    }
}

void Serialization::SerializeString(std::ofstream& output, const std::string& string_to_serialize) {
    size_t name_length = string_to_serialize.size();
    output.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
    output.write(string_to_serialize.data(), name_length);
}

std::string Serialization::DeserializeString(std::ifstream& input) {
    size_t name_length = DeserializeValue<size_t>(input);
    std::string str;
    str.resize(name_length);
    input.read(reinterpret_cast<char*>(&str[0]), name_length);
    return str;
}

void Serialization::SerializeColor(std::ofstream& output, const svg::Color& color) {
    if (std::holds_alternative<std::monostate>(color)) {
        int x = 1;
        output.write(reinterpret_cast<const char*>(&x), sizeof(x));
    }
    else if (std::holds_alternative<std::string>(color)) {
        int x = 2;
        output.write(reinterpret_cast<const char*>(&x), sizeof(x));
        size_t name_length = std::get<std::string>(color).size();
        output.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
        const std::string& temp_str = std::get<std::string>(color);
        output.write(temp_str.data(), name_length);
    }
    else if (std::holds_alternative<svg::Rgb>(color)) {
        int x = 3;
        output.write(reinterpret_cast<const char*>(&x), sizeof(x));
        const svg::Rgb& temp_rgb = std::get<svg::Rgb>(color);
        output.write(reinterpret_cast<const char*>(&temp_rgb), sizeof(temp_rgb));
    }
    else if (std::holds_alternative<svg::Rgba>(color)) {
        int x = 4;
        output.write(reinterpret_cast<const char*>(&x), sizeof(x));
        const svg::Rgba& temp_rgba = std::get<svg::Rgba>(color);
        output.write(reinterpret_cast<const char*>(&temp_rgba), sizeof(temp_rgba));
    }
}

svg::Color Serialization::DeserializeColor(std::ifstream& input) {
    int x = 0;
    input.read(reinterpret_cast<char*>(&x), sizeof(x));

    if (x == 1) {
        return std::monostate{};
    }
    else if (x == 2) {
        size_t length = 0;
        input.read(reinterpret_cast<char*>(&length), sizeof(length));
        std::string dst;
        dst.resize(length);
        input.read(reinterpret_cast<char*>(&dst[0]), length);

        return dst;
    }
    else if (x == 3) {
        svg::Rgb dst;
        input.read(reinterpret_cast<char*>(&dst), sizeof(dst));

        return dst;
    }
    else if (x == 4) {
        svg::Rgba dst;
        input.read(reinterpret_cast<char*>(&dst), sizeof(dst));

        return dst;
    }
    else {
        throw;
    }
}