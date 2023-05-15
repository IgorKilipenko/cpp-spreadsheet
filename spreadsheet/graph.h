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

    struct Edge {
        VertexId from;
        VertexId to;
        bool operator==(const Edge& other) const {
            return from == other.from && to == other.to;
        }
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
            return this->operator()(edge.from) + this->operator()(edge.to) * INDEX;
        }

    private:
        std::hash<const void*> pointer_hasher_;
        static const size_t INDEX = 42;
    };

    class Graph {
    public:
        using IncidenceList = std::unordered_set<Edge, Hasher>;
        using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;
        using EdgeContainer = std::unordered_set<Edge, Hasher>;
        using IncidentEdges = std::unordered_map<VertexId, IncidenceList, Hasher>;

    public:
        Graph() = default;

        Graph(EdgeContainer&& edges, IncidentEdges&& incidence_lists);

        bool AddEdge(Edge edge);
        bool HasEdge(const Edge& edge) const;
        size_t GetVertexCount() const;
        size_t GetEdgeCount() const;
        IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;
        bool EraseEdge(const Edge& edge);
        bool DetectCircularDeps(const VertexId& vertex_id) const;

    private:
        EdgeContainer edges_;
        IncidentEdges incidence_lists_;
    };

    inline Graph::Graph(EdgeContainer&& edges, IncidentEdges&& incidence_lists)
        : edges_(std::move(edges)), incidence_lists_(std::move(incidence_lists)) {}

    inline bool Graph::AddEdge(Edge edge) {
        auto emplaced_edge = edges_.emplace(edge);
        if (!emplaced_edge.second) {
            return false;
        }

        incidence_lists_[emplaced_edge.first->from].emplace(*emplaced_edge.first);
        return true;
    }

    inline bool Graph::HasEdge(const Edge& edge) const {
        return edges_.count(edge) != 0;
    }

    inline bool Graph::EraseEdge(const Edge& edge) {
        if (!edges_.erase(edge)) {
            return false;
        }

        incidence_lists_.erase(edge.from);
        return true;
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

    bool Graph::DetectCircularDeps(const VertexId& vertex_id) const {
        /*const auto incident_edge_it = incidence_lists_.find(vertex_id);
        if (incident_edge_it == incidence_lists_.end() || incident_edge_it->second.empty()) {
            return false;
        }

        IncidenceList resolved_edges;
        const auto resolve =[&] (const Edge& edge) {
            if (resolved_edges.count(edge)) {
                return false;
            }

            resolved_edges.emplace(edge);
            
        };*/

        return false;
    }
}