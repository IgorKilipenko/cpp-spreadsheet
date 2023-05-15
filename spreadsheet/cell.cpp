#include "cell.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>

#include "common.h"

Cell::Cell(SheetInterface& sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_{sheet} {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    } else if (text.length() > 1 && text[0] == '=') {
        std::unique_ptr<FormulaImpl> impl_temp = std::make_unique<FormulaImpl>(std::move(text.erase(0, 1)), sheet_);
        std::vector<Position> cell_refs = impl_temp->GetReferencedCells();
        if (DetectCircularDeps_(cell_refs)) {
            throw CircularDependencyException("Has circular dependencies");
        }
        std::for_each(cell_refs.begin(), cell_refs.end(), [&](const Position& pos) {
            if (!sheet_.GetCell(pos)) {
                sheet_.SetCell(pos, "");
            }
        });
        impl_ = std::move(impl_temp);
    } else {
        impl_ = std::make_unique<TextImpl>(std::move(text));
    }
}

void Cell::Clear() {
    impl_ = nullptr;
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}