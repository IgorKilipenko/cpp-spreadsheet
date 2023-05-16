#pragma once

#include <algorithm>
#include <cstddef>
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
        std::size_t operator()(const Position& pos) const {
            return std::hash<int>()(pos.row) + std::hash<int>()(pos.col) * INDEX;
        }

        std::size_t operator()(const Edge& edge) const {
            return this->operator()(edge.from) + this->operator()(edge.to) * INDEX;
        }

        size_t operator()(const Edge* item) const {
            return this->operator()(*item);
        }

        template <typename T>
        size_t operator()(std::initializer_list<const T*> items) const {
            size_t hash = 0;
            for (const T* cur : items) {
                hash = hash * INDEX + pointer_hasher_(cur);
            }
            return hash;
        }

    private:
        std::hash<const void*> pointer_hasher_;
        static const size_t INDEX = 42;
    };

    class DirectedGraph {
    public:
        using IncidenceList = std::unordered_set<const Edge*, Hasher>;
        using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;
        using EdgeContainer = std::unordered_set<Edge, Hasher>;
        using IncidentEdges = std::unordered_map<VertexId, IncidenceList, Hasher>;

    public:
        DirectedGraph() = default;

        DirectedGraph(EdgeContainer&& edges, IncidentEdges&& incidence_lists);

        bool AddEdge(Edge edge);
        template <typename It, std::enable_if_t<std::is_same_v<typename std::iterator_traits<It>::value_type, Edge>, bool> = true>
        size_t AddEdges(It begin, It end);
        bool HasEdge(const Edge& edge) const;
        size_t GetVertexCount() const;
        size_t GetEdgeCount() const;
        IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;
        bool EraseEdge(const Edge& edge);
        bool EraseVertex(const VertexId& vertex_id);
        // void Traverse(const VertexId& vertex_id);

    private:
        EdgeContainer edges_;
        IncidentEdges incidence_lists_;
    };

    inline DirectedGraph::DirectedGraph(EdgeContainer&& edges, IncidentEdges&& incidence_lists)
        : edges_(std::move(edges)), incidence_lists_(std::move(incidence_lists)) {}

    inline bool DirectedGraph::AddEdge(Edge edge) {
        auto emplaced_edge = edges_.emplace(std::move(edge));
        if (!emplaced_edge.second) {
            return false;
        }

        incidence_lists_[emplaced_edge.first->from].emplace(&*emplaced_edge.first);
        return true;
    }

    template <typename It, std::enable_if_t<std::is_same_v<typename std::iterator_traits<It>::value_type, Edge>, bool>>
    size_t DirectedGraph::AddEdges(It begin, It end) {
        size_t count = 0;
        std::for_each(std::move_iterator(begin), std::move_iterator(end), [&](auto&& edge) {
            count += AddEdge(std::forward<decltype(edge)>(edge)) ? 1 : 0;
        });
        return count;
    }

    inline bool DirectedGraph::HasEdge(const Edge& edge) const {
        return edges_.count(edge) != 0;
    }

    inline bool DirectedGraph::EraseEdge(const Edge& edge) {
        if (!edges_.erase(edge)) {
            return false;
        }

        incidence_lists_.erase(edge.from);
        return true;
    }

    inline size_t DirectedGraph::GetVertexCount() const {
        return incidence_lists_.size();
    }

    inline size_t DirectedGraph::GetEdgeCount() const {
        return edges_.size();
    }

    inline typename DirectedGraph::IncidentEdgesRange DirectedGraph::GetIncidentEdges(VertexId vertex) const {
        return ranges::AsRange(incidence_lists_.at(vertex));
    }

    inline bool DirectedGraph::EraseVertex(const VertexId& vertex_id) {
        const auto incidence_it = incidence_lists_.find(vertex_id);
        if (incidence_it == incidence_lists_.end()) {
            return false;
        }
        if (incidence_lists_.size() == 1) {
            edges_.clear();
        } else {
            for (auto ptr = edges_.begin(), last = edges_.end(); ptr != last;) {
                if (incidence_it->second.count(&*ptr)) {
                    ptr = edges_.erase(ptr);
                } else {
                    ++ptr;
                }
            }
        }
        incidence_lists_.erase(incidence_it);
        return true;
    }
}