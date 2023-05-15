#pragma once

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "common.h"
#include "ranges.h"

namespace graph {

    using VertexId = Position;
    using EdgeId = Position;

    struct Edge {
        VertexId from;
        VertexId to;
    };

    struct Hasher {
        template <typename T>
        size_t operator()(std::initializer_list<const T*> items) const {
            size_t hash = 0;
            for (const T* cur : items) {
                hash = hash * INDEX + pointer_hasher_(cur);
            }
            return hash;
        }

        std::size_t operator()(const Position& pos) const {
            return std::hash<int>()(pos.row) + std::hash<int>()(pos.col) * INDEX;
        }

        std::size_t operator()(const Edge& edge) const {
            return this->operator()(edge.from) + this->operator()(edge.to);
        }

    private:
        std::hash<const void*> pointer_hasher_;
        static const size_t INDEX = 42;
    };

    class Graph {
    public:
        using IncidenceList = std::unordered_set<EdgeId, Hasher>;
        using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;
        using EdgeContainer = std::unordered_map<EdgeId, Edge, Hasher, std::less<EdgeId>>;
        using IncidentEdges = std::unordered_map<VertexId, IncidenceList, Hasher>;

    public:
        Graph() = default;

        Graph(EdgeContainer&& edges, IncidentEdges&& incidence_lists);

        template <typename TEdge, std::enable_if_t<std::is_same_v<std::decay_t<TEdge>, Edge>, bool> = true>
        EdgeId AddEdge(TEdge&& edge);
        bool HasEdge(const EdgeId& id) const;
        const Edge* GetEdge(const EdgeId& id) const;
        size_t GetVertexCount() const;
        size_t GetEdgeCount() const;
        IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;
        template <typename TEdge, std::enable_if_t<std::is_same_v<std::decay_t<TEdge>, Edge>, bool> = true>
        bool EraseEdge(TEdge&& edge);
        template <typename TEdgeId, std::enable_if_t<std::is_same_v<std::decay_t<TEdgeId>, EdgeId>, bool> = true>
        bool EraseEdge(TEdgeId&& edge_id);

    private:
        EdgeContainer edges_;
        IncidentEdges incidence_lists_;
    };

    inline Graph::Graph(EdgeContainer&& edges, IncidentEdges&& incidence_lists)
        : edges_(std::move(edges)), incidence_lists_(std::move(incidence_lists)) {}

    template <typename TEdge, std::enable_if_t<std::is_same_v<std::decay_t<TEdge>, Edge>, bool>>
    EdgeId Graph::AddEdge(TEdge&& edge) {
        edges_[edge.from] = edge;

        incidence_lists_[edge.from].emplace(edge.to);
        return edge.from;
    }

    inline bool Graph::HasEdge(const EdgeId& edge_id) const {
        return edges_.count(edge_id) != 0;
    }

    template <typename TEdge, std::enable_if_t<std::is_same_v<std::decay_t<TEdge>, Edge>, bool>>
    bool Graph::EraseEdge(TEdge&& edge) {
        if (!edges_.erase(edge.from)) {
            return false;
        }

        incidence_lists_.erase(edge.from);
        return true;
    }

    template <typename TEdgeId, std::enable_if_t<std::is_same_v<std::decay_t<TEdgeId>, EdgeId>, bool>>
    bool Graph::EraseEdge(TEdgeId&& edge_id) {
        const Edge* edge = GetEdge(edge_id);
        if (edge == nullptr) {
            return false;
        }
        return EraseEdge(*edge);
    }

    inline size_t Graph::GetVertexCount() const {
        return incidence_lists_.size();
    }

    inline size_t Graph::GetEdgeCount() const {
        return edges_.size();
    }

    inline typename Graph::IncidentEdgesRange Graph::GetIncidentEdges(VertexId vertex) const {
        return ranges::AsRange(incidence_lists_.at(vertex));
    }

    inline const Edge* Graph::GetEdge(const EdgeId& id) const {
        auto edge_it = edges_.find(id);
        return edge_it == edges_.end() ? nullptr : &edge_it->second;
    }
}