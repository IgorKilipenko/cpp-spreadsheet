#include "cell.h"

#include <cassert>
#include <memory>
#include <string>
#include <utility>

#include "common.h"

Cell::Cell(SheetInterface& sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_{sheet} {}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    ClearCache();

    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    } else if (text.length() > 1 && text[0] == '=') {
        std::unique_ptr<FormulaImpl> impl_temp = std::make_unique<FormulaImpl>(std::move(text.erase(0, 1)), sheet_);
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
    assert(impl_ != nullptr);
    return impl_->GetReferencedCells();
}

void Cell::ClearCache() {
    cache_ = nullptr;
}

bool Cell::HasCache() const {
    return cache_ != nullptr;
}
