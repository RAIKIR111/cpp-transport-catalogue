#pragma once

#include "router.h"

class TransportRouter {
public:
    TransportRouter() = default;

    TransportRouter(int bus_wait_time, double bus_velocity);

    void SetBusWaitTime(int bus_wait_time);

    void SetBusVelocity(int bus_velocity);

    int GetBusWaitTime() const;

    double GetBusVelocity() const;

    void UpdStopnameToVertex(std::string_view stopname, int vertex);

    int GetVertexByStopname(std::string_view stopname) const;

    void UpdEdgesData(graph::Edge<double> edge, std::string_view name, int span_count);

    const std::pair<std::string_view, int>& GetEdgeData(const graph::Edge<double>& edge) const;

private:
    struct EdgeHasher {
        size_t operator()(const graph::Edge<double>& edge) const {
            return std::hash<size_t>()(edge.from) + std::hash<size_t>()(edge.to * 57) + std::hash<double>()(edge.weight) * 57 * 57;
        }
    };

    int bus_wait_time_ = 0;
    double bus_velocity_ = 0;
    std::unordered_map<std::string_view, int> stopname_to_vertex_;
    std::unordered_map<graph::Edge<double>, std::pair<std::string_view, int>, EdgeHasher> edge_to_data_;
};