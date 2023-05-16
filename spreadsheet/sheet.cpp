#include "sheet.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <variant>

#include "cell.h"
#include "common.h"

namespace spreadsheet /* Sheet implementation public methods */ {

    using namespace std::literals;

    void Sheet::SetCell(Position pos, std::string text) {
        ValidatePosition_(pos);

        size_.rows = pos.row - size_.rows >= 0 ? pos.row + 1 : size_.rows;
        size_.cols = pos.col - size_.cols >= 0 ? pos.col + 1 : size_.cols;

        auto row_it = sheet_.find(pos.row);
        row_it = row_it != sheet_.end() ? row_it : sheet_.emplace(pos.row, ColumnItem()).first;

        if (const auto cell_it = row_it->second.find(pos.col); cell_it != row_it->second.end()) {
            if (cell_it->second->GetText() != text) {
                cell_it->second->Set(std::move(text));
            }
        } else {
            auto value = std::make_unique<Cell>(*this);
            value->Set(std::move(text));
            row_it->second.emplace(pos.col, std::move(value));
            BuildGraph_(pos);
        }
    }

    const CellInterface* Sheet::GetCell(Position pos) const {
        ValidatePosition_(pos);
        return GetCell_(std::move(pos));
    }

    CellInterface* Sheet::GetCell(Position pos) {
        ValidatePosition_(pos);
        return const_cast<Cell*>(GetCell_(std::move(pos)));
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

    void Sheet::BuildGraph_(const Position& position) {
        std::function<void(const Position&)> build;

        std::unordered_set<Position, graph::Hasher> visited;
        build = [&](const Position& from) {
            CellInterface* cell = GetCell(from);
            const auto refs = cell->GetReferencedCells();
            std::for_each(refs.begin(), refs.end(), [&](const Position& to) {
                if (visited.count(from) > 0) {
                    return;
                }

                graph_.AddEdge({from, to});
                build(to);
            });
            visited.emplace(from);
        };

        build(position);
    }

    const graph::Graph& Sheet::GetGraph() const {
        return graph_;
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<spreadsheet::Sheet>();
}