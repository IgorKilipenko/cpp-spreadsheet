#include "cell.h"

#include <cassert>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>

// Реализуйте следующие методы
Cell::Cell() : impl_(std::make_unique<EmptyImpl>()) {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    } else if (text.length() > 1 && text[0] == '=') {
        impl_ = std::make_unique<FormulaImpl>(std::move(text.erase(0, 1)));
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