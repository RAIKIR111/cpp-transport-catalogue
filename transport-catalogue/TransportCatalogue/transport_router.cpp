#include "transport_router.h"

using namespace std;
//using namespace 

TransportRouter::TransportRouter(int bus_wait_time, double bus_velocity)
    : bus_wait_time_(bus_wait_time)
    , bus_velocity_(bus_velocity) {
}

void TransportRouter::SetBusWaitTime(int bus_wait_time) {
    bus_wait_time_ = bus_wait_time;
}

void TransportRouter::SetBusVelocity(int bus_velocity) {
    bus_velocity_ = bus_velocity;
}

int TransportRouter::GetBusWaitTime() const {
    return bus_wait_time_;
}

double TransportRouter::GetBusVelocity() const {
    return bus_velocity_;
}

void TransportRouter::UpdStopnameToVertex(std::string stopname, int vertex) {
    stopname_to_vertex_[stopname] = vertex;
}

int TransportRouter::GetVertexByStopname(std::string_view stopname) const {
    return stopname_to_vertex_.at(string(stopname));
}

void TransportRouter::UpdEdgesData(graph::Edge<double> edge, std::string name, int span_count) {
    edge_to_data_[edge] = std::make_pair(name, span_count);
}

const std::pair<std::string, int>& TransportRouter::GetEdgeData(const graph::Edge<double>& edge) const {
    return edge_to_data_.at(edge);
}

const std::unordered_map<std::string, int>& TransportRouter::GetFullStopnameToVertex() const {
    return stopname_to_vertex_;
}

const std::unordered_map<graph::Edge<double>, std::pair<std::string, int>, TransportRouter::EdgeHasher>& TransportRouter::GetFullEdgeToData() const {
    return edge_to_data_;
}