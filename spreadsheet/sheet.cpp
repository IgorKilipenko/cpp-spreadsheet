#include "sheet.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <variant>

#include "cell.h"
#include "common.h"

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    ValidatePosition_(pos);

    size_.rows = pos.row - size_.rows >= 0 ? pos.row + 1 : size_.rows;
    size_.cols = pos.col - size_.cols >= 0 ? pos.col + 1 : size_.cols;

    auto row_it = sheet_.find(pos.row);
    row_it = row_it != sheet_.end() ? row_it : sheet_.emplace(pos.row, Column()).first;

    if (const auto row_it = sheet_.find(pos.row); row_it != sheet_.end()) {
        if (const auto cell_it = row_it->second.find(pos.col); cell_it != row_it->second.end()) {
            cell_it->second->Set(std::move(text));
            return;
        } else {
            auto value = std::make_unique<Cell>();
            value->Set(std::move(text));
            row_it->second.emplace(pos.col, std::move(value));
        }
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

    if (IsLastPosition_(pos)) {
        size_ = CalculateSize_();
    }
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValue_(std::ostream& output, const Cell::Value& value) const {
    if (auto error_ptr = std::get_if<FormulaError>(&value); error_ptr != nullptr) {
        output << *error_ptr;
    } else if (auto num_ptr = std::get_if<double>(&value); num_ptr != nullptr) {
        output << *num_ptr;
    } else {
        output << *std::get_if<std::string>(&value);
    }
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        for (int j = 0; j < size_.cols; ++j) {
            auto cell = GetCell({i, j});
            if (cell != nullptr) {
                PrintValue_(output, cell->GetValue());
            }

            if (j + 1 != size_.cols) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        for (int j = 0; j < size_.cols; ++j) {
            auto cell = GetCell({i, j});
            if (cell != nullptr) {
                output << cell->GetText();
            }

            if (j + 1 != size_.cols) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

Size Sheet::CalculateSize_() const {
    Size size{-1, -1};
    for (const auto& [row, col_map] : sheet_) {
        size.rows = size.rows < row ? row : size.rows;
        for (const auto& [col, _] : col_map) {
            size.cols = size.cols < col ? col : size.cols;
        }
    }
    return {size.rows + 1, size.cols + 1};
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}