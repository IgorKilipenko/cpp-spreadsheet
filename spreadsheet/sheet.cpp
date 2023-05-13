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

    auto cell_it = sheet_.find(pos);

    if (cell_it != sheet_.end()) {
        cell_it->second->Set(std::move(text));
    } else {
        auto cell_ptr = std::make_unique<Cell>();
        cell_ptr->Set(std::move(text));
        sheet_.emplace(std::move(pos), std::move(cell_ptr));
    }
}

const CellInterface* Sheet::GetCell_(Position&& pos) const {
    ValidatePosition_(pos);

    auto cell_ptr = sheet_.find(pos);

    if (cell_ptr == sheet_.end()) {
        return nullptr;
    }

    return cell_ptr->second.get();
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return GetCell_(std::move(pos));
}

CellInterface* Sheet::GetCell(Position pos) {
    return const_cast<CellInterface*>(GetCell_(std::move(pos)));
}

void Sheet::ClearCell(Position pos) {
    ValidatePosition_(pos);

    if (!GetCell(pos)) {
        return;
    }

    sheet_.erase(pos);

    if (size_.rows - pos.row == 1 || size_.cols - pos.col == 1) {
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
    Size new_size{-1, -1};
    for (const auto& [pos, _] : sheet_) {
        if (new_size.rows < pos.row) {
            new_size.rows = pos.row;
        }
        if (new_size.cols < pos.col) {
            new_size.cols = pos.col;
        }
    }
    return {new_size.rows + 1, new_size.cols + 1};
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}