#include "cell.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include "common.h"

Cell::Cell(SheetInterface& sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_{sheet} {}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    ClearCache();

    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
        cell_refs_ = {};
    } else if (text.length() > 1 && text[0] == '=') {
        std::unique_ptr<FormulaImpl> impl_temp = std::make_unique<FormulaImpl>(std::move(text.erase(0, 1)), sheet_);
        cell_refs_ = impl_temp->GetReferencedCells();
        impl_ = std::move(impl_temp);
    } else {
        impl_ = std::make_unique<TextImpl>(std::move(text));
    }
}

void Cell::Clear() {
    cache_ = nullptr;
    impl_ = nullptr;
}

Cell::Value Cell::GetValue() const {
    assert(impl_ != nullptr);

    if (!HasCache()) {
        cache_ = std::make_unique<Value>(impl_->GetValue());
    }

    return *cache_;
}

std::string Cell::GetText() const {
    assert(impl_ != nullptr);

    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return cell_refs_;
}

const std::vector<Position>& Cell::GetStoredReferencedCells() const {
    return cell_refs_;
}

void Cell::ClearCache() {
    cache_ = nullptr;
}

bool Cell::HasCache() const {
    return cache_ != nullptr;
}

bool Cell::IsReferenced() const {
    return !cell_refs_.empty();
}