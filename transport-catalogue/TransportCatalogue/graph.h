#pragma once

#include "ranges.h"

#include <cstdlib>
#include <vector>

namespace graph {

using VertexId = size_t;
using EdgeId = size_t;

template <typename Weight>
struct Edge {
    Edge() = default;

    Edge(VertexId e_from, VertexId e_to, Weight e_weight)
        : from(e_from)
        , to(e_to)
        , weight(e_weight) {
    }

    VertexId from;
    VertexId to;
    Weight weight;

    bool operator==(const Edge& entry) const {
        return (from == entry.from) && (to == entry.to) && (weight == entry.weight);
    }
};

template <typename Weight>
class DirectedWeightedGraph {
private:
    using IncidenceList = std::vector<EdgeId>;
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

public:
    DirectedWeightedGraph() = default; // конструктор графа
    DirectedWeightedGraph(const std::vector<Edge<Weight>>& edges, const std::vector<IncidenceList>& incidence_lists);
    explicit DirectedWeightedGraph(size_t vertex_count); // конструктор графа
    EdgeId AddEdge(const Edge<Weight>& edge); // добавить ребро

    size_t GetVertexCount() const; // получить количество вершин
    size_t GetEdgeCount() const; // получить количество ребер
    const Edge<Weight>& GetEdge(EdgeId edge_id) const; // получить ребро
    IncidentEdgesRange GetIncidentEdges(VertexId vertex) const; // получить рендж (охват) ребер для вершины ???

    const std::vector<std::vector<EdgeId>>& GetFullIncidenceLists() const;

private:
    std::vector<Edge<Weight>> edges_;
    std::vector<IncidenceList> incidence_lists_; // от остановки до ребра
};

template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
    : incidence_lists_(vertex_count) {
}

template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(const std::vector<Edge<Weight>>& edges, const std::vector<IncidenceList>& incidence_lists)
    : edges_(edges)
    , incidence_lists_(incidence_lists) {
}

template <typename Weight>
const std::vector<std::vector<EdgeId>>& DirectedWeightedGraph<Weight>::GetFullIncidenceLists() const {
    return incidence_lists_;
}

template <typename Weight>
EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
    edges_.push_back(edge);
    const EdgeId id = edges_.size() - 1;
    incidence_lists_.at(edge.from).push_back(id);
    return id;
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
    return incidence_lists_.size();
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
    return edges_.size();
}

template <typename Weight>
const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
    return edges_.at(edge_id);
}

template <typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
    return ranges::AsRange(incidence_lists_.at(vertex));
}
}  // namespace graph