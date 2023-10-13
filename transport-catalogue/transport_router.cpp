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

void TransportRouter::UpdStopnameToVertex(std::string_view stopname, int vertex) {
    stopname_to_vertex_[stopname] = vertex;
}

int TransportRouter::GetVertexByStopname(std::string_view stopname) const {
    return stopname_to_vertex_.at(stopname);
}

void TransportRouter::UpdEdgesData(graph::Edge<double> edge, std::string_view name, int span_count) {
    edge_to_data_[edge] = std::make_pair(name, span_count);
}

const std::pair<std::string_view, int>& TransportRouter::GetEdgeData(const graph::Edge<double>& edge) const {
    return edge_to_data_.at(edge);
}