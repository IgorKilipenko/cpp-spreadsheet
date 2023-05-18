#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "cell.h"
#include "common.h"

namespace graph /* IGraph */ {
    
    using VertexId = Position;

    struct Edge {
        VertexId from;
        VertexId to;
        bool operator==(const Edge& other) const;
    };

    struct Hasher {
        std::size_t operator()(const Position& pos) const;
        std::size_t operator()(const Edge& edge) const;
        size_t operator()(const Edge* item) const;

    private:
        static const size_t INDEX = 42;
    };

    using IncidenceList = std::unordered_set<const Edge*, Hasher>;
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;
    using EdgeContainer = std::unordered_set<Edge, Hasher>;
    using IncidentEdges = std::unordered_map<VertexId, IncidenceList, Hasher>;

    class IGraph {
    public:
        virtual bool AddEdge(Edge edge) = 0;
        virtual bool HasEdge(const Edge& edge) const = 0;
        virtual size_t GetVertexCount() const = 0;
        virtual size_t GetEdgeCount() const = 0;
        virtual IncidentEdgesRange GetIncidentEdges(VertexId vertex) const = 0;
        virtual bool EraseEdge(const Edge& edge) = 0;
        virtual bool EraseVertex(const VertexId& vertex_id) = 0;
        virtual void Traversal(const VertexId& vertex_id, std::function<bool(const Edge*)> action) const = 0;
        virtual bool DetectCircularDependency(const VertexId& from, const std::vector<VertexId>& to_refs) const = 0;

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

namespace graph /* DependencyGraph */ {

    class DependencyGraph final : IGraph {
    public:
        DependencyGraph();
        DependencyGraph(DirectedGraph forward_graph, DirectedGraph backward_graph);
        ~DependencyGraph();

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

namespace spreadsheet /* Sheet definations */ {

    class Sheet : public SheetInterface {
    private:
        using ColumnItem = std::unordered_map<int, std::unique_ptr<Cell>>;

    public:
        Sheet() = default;
        ~Sheet() = default;

    public:
        void SetCell(Position pos, std::string text) override;

        const Cell* GetCell(Position pos) const override;
        Cell* GetCell(Position pos) override;

        void ClearCell(Position pos) override;

        Size GetPrintableSize() const override;

        void PrintValues(std::ostream& output) const override;
        void PrintTexts(std::ostream& output) const override;

        const graph::DependencyGraph& GetGraph() const;

    private:
        template <typename TPosition, std::enable_if_t<std::is_same_v<std::decay_t<TPosition>, Position>, bool> = true>
        const Cell* GetConstCell_(TPosition&& pos) const;
        void ValidatePosition_(const Position& pos) const;
        void CalculateSize_(Position&& erased_pos);
        void Print_(std::ostream& output, std::function<void(const CellInterface*)> print) const;
        void InvalidateCache_(const Position& pos);

    private:
        std::unordered_map<int, ColumnItem> sheet_;
        Size size_ = {0, 0};
        graph::DependencyGraph graph_;
    };
}

namespace spreadsheet /* Sheet template implementation */ {

    template <typename TPosition, std::enable_if_t<std::is_same_v<std::decay_t<TPosition>, Position>, bool>>
    const Cell* Sheet::GetConstCell_(TPosition&& pos) const {
        if (const auto row_ptr = sheet_.find(pos.row); row_ptr != sheet_.end()) {
            if (const auto cell_ptr = row_ptr->second.find(pos.col); cell_ptr != row_ptr->second.end()) {
                return cell_ptr->second.get();
            }
        }
        return nullptr;
    }
}