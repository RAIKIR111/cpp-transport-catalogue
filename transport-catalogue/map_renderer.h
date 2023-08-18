#pragma once

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

#include "geo.h"
#include "svg.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <set>
#include <deque>

namespace map_renderer {

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

struct Settings {
    double width = 0.0;
    double height = 0.0;
    double padding = 0.0;
    double line_width = 0.0;
    double stop_radius = 0.0;
    int bus_label_font_size = 0;
    std::vector<double> bus_label_offset;
    int stop_label_font_size = 0;
    std::vector<double> stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width_ = 0.0;
    std::vector<svg::Color> color_palette;    
};

class MapRenderer {
public:
    struct CompareStopsPairsByName {
        bool operator()(const std::pair<std::string_view, svg::Point> p1, const std::pair<std::string_view, svg::Point> p2) const {
            return p1.first < p2.first;
        }
    };

    struct CompareBusesByName {
        bool operator()(const domain::Bus* bus1, const domain::Bus* bus2) const {
            return bus1->name <= bus2->name;
        }
    };

    MapRenderer();

    void SetWidth(double x);
    void SetHeight(double x);
    void SetPadding(double x);
    void SetLineWidth(double x);
    void SetStopRadius(double x);
    void SetBusLabelFontSize(int x);
    void SetBusLabelOffset(std::vector<double> x);
    void SetStopLabelFontSize(int x);
    void SetStopLabelOffset(std::vector<double> x);
    void SetUnderlayerColor(svg::Color x);
    void SetUnderlayerWidth(double x);
    void SetColorPalette(std::vector<svg::Color> x);

    const Settings* GetSettings() const;

    svg::Document Render(const std::deque<domain::Bus>& buses, const std::vector<domain::Stop*>& active_stops, const std::unordered_map<std::string_view, bool, domain::StringViewHasher>&) const;

private:
    Settings* settings_ = nullptr;

    const svg::Polyline ConstructRouteLines(const domain::Bus* bus, const map_renderer::SphereProjector& projector, int color_counter, std::set<std::pair<std::string_view, svg::Point>, CompareStopsPairsByName>& stop_circles_pair_vec) const;

    const svg::Text ConstructBusSubText(const std::string_view data, const svg::Point& point) const;

    const svg::Text ConstructBusMainText(const std::string_view data, const svg::Point& point, int color_counter) const;

    const svg::Text ConstructStopSubText(const std::string_view data, const svg::Point& point) const;

    const svg::Text ConstructStopMainText(const std::string_view data, const svg::Point& point) const;

    const svg::Circle ConstructCircle(const svg::Point& point) const;

    const std::set<const domain::Bus*, CompareBusesByName> GetBusesInNameOrder(const std::deque<domain::Bus>& buses) const;

    const std::vector<geo::Coordinates> GetCoordinatesOfActiveStops(const std::vector<domain::Stop*>& active_stops) const;
};

} // namespace map_renderer