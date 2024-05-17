#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <functional>
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

    using IncidenceList = std::unordered_set<const Edge*, Hasher>;
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;
    using EdgeContainer = std::unordered_set<Edge, Hasher>;
    using IncidentEdges = std::unordered_map<VertexId, IncidenceList, Hasher>;
}

namespace graph /* IGraph */ {
    class IGraph {
    public:
        virtual bool AddEdge(Edge edge) = 0;
        [[nodiscard]] virtual bool HasEdge(const Edge& edge) const = 0;
        [[nodiscard]] virtual size_t GetVertexCount() const = 0;
        [[nodiscard]] virtual size_t GetEdgeCount() const = 0;
        [[nodiscard]] virtual IncidentEdgesRange GetIncidentEdges(VertexId vertex) const = 0;
        virtual bool EraseEdge(const Edge& edge) = 0;
        virtual bool EraseVertex(const VertexId& vertex_id) = 0;
        virtual void Traversal(const VertexId& vertex_id, std::function<bool(const Edge*)> action) const = 0;
        [[nodiscard]] virtual bool DetectCircularDependency(const VertexId& from, const std::vector<VertexId>& to_refs) const = 0;

    protected:
        virtual size_t AddEdgesImpl(EdgeContainer::iterator begin, EdgeContainer::iterator end) = 0;
    };
}

namespace graph /* DirectedGraph */ {

    class DependencyGraph;

    class DirectedGraph : IGraph {
        friend DependencyGraph;

    public:
        DirectedGraph() = default;
        ~DirectedGraph() = default;

        DirectedGraph(EdgeContainer&& edges, IncidentEdges&& incidence_lists);

        bool AddEdge(Edge edge) override;
        template <typename It, std::enable_if_t<std::is_same_v<typename std::iterator_traits<It>::value_type, Edge>, bool> = true>
        size_t AddEdges(It begin, It end);
        bool HasEdge(const Edge& edge) const override;
        size_t GetVertexCount() const override;
        size_t GetEdgeCount() const override;
        IncidentEdgesRange GetIncidentEdges(VertexId vertex) const override;
        virtual bool EraseEdge(const Edge& edge) override;
        virtual bool EraseVertex(const VertexId& vertex_id) override;
        void Traversal(const VertexId& vertex_id, std::function<bool(const Edge*)> action) const override;
        bool DetectCircularDependency(const VertexId& from, const std::vector<VertexId>& to_refs) const override;

    protected:
        EdgeContainer edges_;
        IncidentEdges incidence_lists_;

    protected:
        size_t AddEdgesImpl(EdgeContainer::iterator begin, EdgeContainer::iterator end) override;
    };
}

namespace graph /* DirectedGraph implementation */ {

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

    inline size_t DirectedGraph::AddEdgesImpl(EdgeContainer::iterator begin, EdgeContainer::iterator end) {
        return AddEdges(std::move_iterator(begin), std::move_iterator(end));
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

    inline IncidentEdgesRange DirectedGraph::GetIncidentEdges(VertexId vertex) const {
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

    inline void DirectedGraph::Traversal(const VertexId& vertex_id, std::function<bool(const Edge*)> action) const {
        const auto incidence_edges_it = incidence_lists_.find(vertex_id);
        if (incidence_edges_it == incidence_lists_.end()) {
            return;
        }
        std::function<void(const VertexId&)> traverse;
        std::unordered_set<VertexId, Hasher> visited;
        traverse = [&](const VertexId& from) {
            const auto incidence_edges_it = incidence_lists_.find(from);
            if (incidence_edges_it == incidence_lists_.end()) {
                return;
            }

            const bool stop_traverse = std::any_of(incidence_edges_it->second.begin(), incidence_edges_it->second.end(), [&](const Edge* edge) {
                if (visited.count(edge->from) > 0) {
                    return false;
                }

                traverse(edge->to);

                visited.emplace(edge->to);
                return action(edge);
            });

            if (stop_traverse) {
                return;
            }
        };

        traverse(vertex_id);
    }

    inline bool DirectedGraph::DetectCircularDependency(const VertexId& from, const std::vector<VertexId>& to_refs) const {
        return std::any_of(to_refs.begin(), to_refs.end(), [&](const VertexId& ref) {
            if (from == ref) {
                return true;
            }

            const auto incidence_edges_it = incidence_lists_.find(ref);
            if (incidence_edges_it == incidence_lists_.end()) {
                return false;
            }

            bool has_circular_dependency = false;
            Traversal(ref, [&](const Edge* edge) -> bool {
                if (from == edge->to) {
                    has_circular_dependency = true;
                    return true;
                }
                return false;
            });
            return has_circular_dependency;
        });
    }
}

namespace graph /* Graph DependencyGraph */ {

    class DependencyGraph final : IGraph {
    public:
        DependencyGraph() = default;
        DependencyGraph(DirectedGraph forward_graph, DirectedGraph backward_graph)
            : forward_graph_(std::move(forward_graph)), backward_graph_(std::move(backward_graph)) {}
        ~DependencyGraph() = default;

    public:
        enum class Direction { forward, backward };

    public:
        bool AddEdge(Edge edge) override;
        template <typename It, std::enable_if_t<std::is_same_v<typename std::iterator_traits<It>::value_type, Edge>, bool> = true>
        size_t AddEdges(It begin, It end);
        bool EraseEdge(const Edge& edge) override;
        bool EraseVertex(const VertexId& vertex_id) override;
        bool HasEdge(const Edge& edge) const override;
        size_t GetVertexCount() const override;
        size_t GetEdgeCount() const override;
        IncidentEdgesRange GetIncidentEdges(VertexId vertex) const override;
        void Traversal(const VertexId& vertex_id, std::function<bool(const Edge*)> action) const override;
        void Traversal(const VertexId& vertex_id, std::function<bool(const Edge*)> action, Direction direction = Direction::forward) const;
        bool DetectCircularDependency(const VertexId& from, const std::vector<VertexId>& to_refs) const override;

    private:
        DirectedGraph forward_graph_;
        DirectedGraph backward_graph_;

    private:
        size_t AddEdgesImpl(EdgeContainer::iterator begin, EdgeContainer::iterator end) override;
    };
}

namespace graph /* Graph implementation */ {

    inline bool DependencyGraph::AddEdge(Edge edge) {
        const bool success = forward_graph_.AddEdge(edge);
        if (success) {
            backward_graph_.AddEdge(Edge{edge.to, edge.from});
        }
        return success;
    }

    template <typename It, std::enable_if_t<std::is_same_v<typename std::iterator_traits<It>::value_type, Edge>, bool>>
    size_t DependencyGraph::AddEdges(It begin, It end) {
        const size_t count = forward_graph_.AddEdges(begin, end);
        if (count == 0) {
            return count;
        }

        std::vector<Edge> tmp(count);
        std::transform(std::move_iterator(begin), std::move_iterator(end), tmp.rbegin(), [](auto&& edge) {
            return Edge{edge.to, edge.from};
        });

        [[maybe_unused]] const size_t back_count = backward_graph_.AddEdges(std::move_iterator(tmp.begin()), std::move_iterator(tmp.end()));
        assert(back_count == count);

        return count;
    }

    inline size_t DependencyGraph::AddEdgesImpl(EdgeContainer::iterator begin, EdgeContainer::iterator end) {
        return AddEdges(std::move(begin), std::move(end));
    }

    inline bool DependencyGraph::EraseEdge(const Edge& edge) {
        const bool success = forward_graph_.EraseEdge(edge);
        if (success) {
            backward_graph_.EraseEdge(Edge{edge.to, edge.from});
        }
        return success;
    }

    inline bool DependencyGraph::EraseVertex(const VertexId& vertex_id) {
        const auto erased_forward_edges_it = forward_graph_.incidence_lists_.find(vertex_id);
        if (erased_forward_edges_it == forward_graph_.incidence_lists_.end()) {
            return false;
        }

        std::for_each(
            erased_forward_edges_it->second.begin(), erased_forward_edges_it->second.end(), [&backward_graph = backward_graph_](const Edge* edge) {
                backward_graph.EraseEdge(Edge{edge->to, edge->from});
            });

        forward_graph_.EraseVertex(vertex_id);

        return true;
    }

    inline bool DependencyGraph::HasEdge(const Edge& edge) const {
        return forward_graph_.HasEdge(edge) || backward_graph_.HasEdge(edge);
    }

    inline size_t DependencyGraph::GetVertexCount() const {
        return forward_graph_.GetVertexCount();
    }

    inline size_t DependencyGraph::GetEdgeCount() const {
        assert(forward_graph_.GetEdgeCount() == backward_graph_.GetEdgeCount());
        return forward_graph_.GetEdgeCount();
    }

    inline IncidentEdgesRange DependencyGraph::GetIncidentEdges(VertexId vertex) const {
        return forward_graph_.GetIncidentEdges(std::move(vertex));
    }

    inline void DependencyGraph::Traversal(const VertexId& vertex_id, std::function<bool(const Edge*)> action) const {
        Traversal(vertex_id, action, Direction::forward);
    }

    inline void DependencyGraph::Traversal(const VertexId& vertex_id, std::function<bool(const Edge*)> action, Direction direction) const {
        if (direction == Direction::forward) {
            forward_graph_.Traversal(vertex_id, action);
        } else {
            backward_graph_.Traversal(vertex_id, action);
        }
    }

    inline bool DependencyGraph::DetectCircularDependency(const VertexId& from, const std::vector<VertexId>& to_refs) const {
        return forward_graph_.DetectCircularDependency(from, to_refs);
    }
}
