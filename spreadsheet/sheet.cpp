#include "sheet.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <variant>
#include <vector>

#include "cell.h"
#include "common.h"
#include "graph.h"

namespace spreadsheet /* Sheet implementation public methods */ {

    using namespace std::literals;

    void Sheet::SetCell(Position pos, std::string text) {
        ValidatePosition_(pos);

        size_.rows = pos.row - size_.rows >= 0 ? pos.row + 1 : size_.rows;
        size_.cols = pos.col - size_.cols >= 0 ? pos.col + 1 : size_.cols;

        auto rows_it = sheet_.find(pos.row);

        std::unique_ptr<Cell> temp_cell;
        const auto cells_it = rows_it != sheet_.end() ? rows_it->second.find(pos.col) : rows_it->second.end();

        if (cells_it != rows_it->second.end() && cells_it->second->GetText() == text) {
            return;
        }

        auto tmp_cell = std::make_unique<Cell>(*this);
        tmp_cell->Set(std::move(text));
        const auto& cell_refs = tmp_cell->GetStoredReferencedCells();

        std::vector<Position> append_positions;
        append_positions.reserve(cell_refs.size());
        std::for_each(cell_refs.begin(), cell_refs.end(), [&](const Position& pos) {
            if (!GetCell(pos)) {
                SetCell(pos, "");
                append_positions.emplace_back(pos);
            }
        });
        BuildGraph_(pos, tmp_cell.get(), [&]() {
            std::for_each(std::move_iterator(append_positions.begin()), std::move_iterator(append_positions.end()), [&](Position&& pos) {
                ClearCell(std::move(pos));
            });
        });

        rows_it = rows_it != sheet_.end() ? rows_it : sheet_.emplace(pos.row, ColumnItem()).first;

        if (cells_it != rows_it->second.end()) {
            cells_it->second = std::move(tmp_cell);
        } else {
            rows_it->second.emplace(pos.col, std::move(tmp_cell));
        }
    }

    const Cell* Sheet::GetCell(Position pos) const {
        ValidatePosition_(pos);
        return GetConstCell_(std::move(pos));
    }

    Cell* Sheet::GetCell(Position pos) {
        ValidatePosition_(pos);
        return const_cast<Cell*>(GetConstCell_(std::move(pos)));
    }

    void Sheet::ClearCell(Position pos) {
        ValidatePosition_(pos);

        if (const auto row_ptr = sheet_.find(pos.row); row_ptr != sheet_.end()) {
            if (row_ptr->second.size() == 1) {
                sheet_.erase(row_ptr);
            } else {
                row_ptr->second.erase(pos.col);
            }
        } else {
            return;
        }

        CalculateSize_(std::move(pos));
    }

    Size Sheet::GetPrintableSize() const {
        return size_;
    }

    void Sheet::PrintValues(std::ostream& output) const {
        Print_(output, [&output](const CellInterface* cell) {
            const auto value = cell->GetValue();
            if (auto error_ptr = std::get_if<FormulaError>(&value); error_ptr != nullptr) {
                output << *error_ptr;
            } else if (auto num_ptr = std::get_if<double>(&value); num_ptr != nullptr) {
                output << *num_ptr;
            } else {
                output << *std::get_if<std::string>(&value);
            }
        });
    }

    void Sheet::PrintTexts(std::ostream& output) const {
        Print_(output, [&output](const CellInterface* cell) {
            output << cell->GetText();
        });
    }
}

namespace spreadsheet /* Sheet implementation private methods */ {

    void Sheet::Print_(std::ostream& output, std::function<void(const CellInterface*)> print_cb) const {
        for (int i = 0; i < size_.rows; ++i) {
            for (int j = 0; j < size_.cols; ++j) {
                if (auto cell = GetCell({i, j}); cell != nullptr) {
                    print_cb(cell);
                }
                if (j + 1 != size_.cols) {
                    output << '\t';
                }
            }
            output << '\n';
        }
    }

    void Sheet::CalculateSize_(Position&& erased_pos) {
        const bool erasedGraterRowIndex = size_.rows - erased_pos.row == 1;
        const bool erasedGraterColIndex = size_.cols - erased_pos.col == 1;
        if (!erasedGraterRowIndex && !erasedGraterColIndex) {
            return;
        }

        Size new_size{-1, -1};
        std::for_each(sheet_.begin(), sheet_.end(), [&new_size, erasedGraterColIndex](const auto& row) {
            new_size.rows = std::max(new_size.rows, row.first);
            new_size.cols = !erasedGraterColIndex ? new_size.cols
                                                  : std::max(
                                                        std::max_element(
                                                            row.second.begin(), row.second.end(),
                                                            [](const auto& lhs, const auto& rhs) {
                                                                return lhs.first < rhs.first;
                                                            })
                                                            ->first,
                                                        new_size.cols);
        });

        size_ = {new_size.rows + 1, new_size.cols + 1};
    }

    void Sheet::ValidatePosition_(const Position& pos) const {
        if (!pos.IsValid()) {
            throw InvalidPositionException("Invalid cell position");
        }
    }

    void Sheet::BuildGraph_(const Position& position, const Cell* cell, std::optional<std::function<void()>> on_error) {
        std::function<void(const Position&, const Cell*)> build_edges;

        graph::DirectedGraph::EdgeContainer edges;
        std::unordered_set<Position, graph::Hasher> visited;
        std::unordered_set<Position, graph::Hasher> seen;
        build_edges = [&](const Position& from, const Cell* cell) {
            const auto refs = cell->GetReferencedCells();
            if (!refs.empty()) {
                seen.emplace(from);
                std::for_each(refs.begin(), refs.end(), [&](const Position& to) {
                    if (visited.count(from) > 0) {
                        return;
                    }

                    if (position == to /*seen.count(to) > 0*/) {
                        if (on_error.has_value()) {
                            on_error.value()();
                        }
                        throw CircularDependencyException("Has circular dependency");
                    }

                    edges.emplace(graph::Edge{from, to});
                    //!------------------------------------------------
                    build_edges(to, GetConstCell_(to));  //! Need Remove
                    visited.emplace(to);
                });
            }
            // visited.emplace(from);
        };

        build_edges(position, cell);
        graph_.EraseVertex(position);
        graph_.AddEdges(std::move_iterator(edges.begin()), std::move_iterator(edges.end()));
    }

    const graph::DirectedGraph& Sheet::GetGraph() const {
        return graph_;
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<spreadsheet::Sheet>();
}