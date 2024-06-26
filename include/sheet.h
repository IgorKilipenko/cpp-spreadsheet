#pragma once

#include <functional>
#include <memory>
#include <type_traits>

#include "cell.h"
#include "common.h"
#include "graph.h"

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
