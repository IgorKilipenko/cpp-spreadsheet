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
        const auto prepare_graph = [&](const std::vector<Position>& refs) {
            InvalidateCache_(pos);
            graph_.EraseVertex(pos);

            std::for_each(refs.begin(), refs.end(), [&](const Position& ref) {
                if (!GetCell(ref)) {
                    SetCell(ref, "");
                }
                graph_.AddEdge({pos, ref});
            });
        };

        ValidatePosition_(pos);

        /// Resize sheet
        size_.rows = pos.row - size_.rows >= 0 ? pos.row + 1 : size_.rows;
        size_.cols = pos.col - size_.cols >= 0 ? pos.col + 1 : size_.cols;

        /// Check cell with this position and value already exists
        auto rows_it = sheet_.find(pos.row);
        const auto cells_it = rows_it != sheet_.end() ? rows_it->second.find(pos.col) : rows_it->second.end();
        if (cells_it != rows_it->second.end() && cells_it->second->GetText() == text) {
            return;
        }

        /// Create temp cell object
        auto tmp_cell = std::make_unique<Cell>(*this);
        tmp_cell->Set(std::move(text));
        const auto& cell_refs = tmp_cell->GetStoredReferencedCells();

        if (graph_.DetectCircularDependency(pos, cell_refs)) {
            throw CircularDependencyException("Has circular dependency");
        }

        /// Build graph (and empty cells if needed)
        prepare_graph(cell_refs);

        /// Append created cell to sheet
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

        InvalidateCache_(pos);
        graph_.EraseVertex(pos);
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

    void Sheet::InvalidateCache_(const Position& pos) {
        graph_.Traversal(pos, [&](const graph::Edge* edge) -> bool {
            Cell* cell = GetCell(edge->to);
            assert(cell != nullptr);

            cell->ClearCache();
            return false;   /// Continue traversal
        }, graph::DependencyGraph::Direction::backward);
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
        const bool grater_row_idx = size_.rows - erased_pos.row == 1;
        const bool grater_col_idx = size_.cols - erased_pos.col == 1;
        if (!grater_row_idx && !grater_col_idx) {
            return;
        }

        Size new_size{-1, -1};
        std::for_each(sheet_.begin(), sheet_.end(), [&new_size, grater_col_idx](const auto& row) {
            new_size.rows = std::max(new_size.rows, row.first);
            new_size.cols = !grater_col_idx ? new_size.cols
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

    const graph::DependencyGraph& Sheet::GetGraph() const {
        return graph_;
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<spreadsheet::Sheet>();
}