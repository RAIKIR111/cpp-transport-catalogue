#pragma once

#include "router.h"

class TransportRouter {
private:
    struct EdgeHasher {
        size_t operator()(const graph::Edge<double>& edge) const {
            return std::hash<size_t>()(edge.from) + std::hash<size_t>()(edge.to * 57) + std::hash<double>()(edge.weight) * 57 * 57;
        }
    };

public:
    TransportRouter() = default;

    TransportRouter(int bus_wait_time, double bus_velocity);

    void SetBusWaitTime(int bus_wait_time);

    void SetBusVelocity(int bus_velocity);

    int GetBusWaitTime() const;

    double GetBusVelocity() const;

    void UpdStopnameToVertex(std::string stopname, int vertex);

    int GetVertexByStopname(std::string_view stopname) const;

    void UpdEdgesData(graph::Edge<double> edge, std::string name, int span_count);

    const std::pair<std::string, int>& GetEdgeData(const graph::Edge<double>& edge) const;

    const std::unordered_map<std::string, int>& GetFullStopnameToVertex() const;

    const std::unordered_map<graph::Edge<double>, std::pair<std::string, int>, EdgeHasher>& GetFullEdgeToData() const;

private:
    int bus_wait_time_ = 0;
    double bus_velocity_ = 0.0;

    std::unordered_map<std::string, int> stopname_to_vertex_;
    std::unordered_map<graph::Edge<double>, std::pair<std::string, int>, EdgeHasher> edge_to_data_;
};