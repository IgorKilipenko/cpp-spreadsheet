#include <doctest/doctest.h>

#include "common.h"

inline Position operator"" _pos(const char* str, std::size_t) {
    return Position::FromString(str);
}

TEST_CASE("Set and Get Cell") {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "Hello, world!");
    CHECK(sheet->GetCell("A1"_pos)->GetText() == "Hello, world!");
}

TEST_CASE("Formula Evaluation") {
    auto sheet = CreateSheet();
    sheet->SetCell(Position::FromString("B1"), "=2+2");
    auto value = sheet->GetCell("B1"_pos)->GetValue();
    CHECK(std::holds_alternative<double>(value));
    CHECK(std::get<double>(value) == 4);
}

TEST_CASE("Clear Cell") {
    auto sheet = CreateSheet();
    sheet->SetCell(Position::FromString("B1"), "=2+2");
    sheet->ClearCell(Position::FromString("B1"));
    CHECK(sheet->GetCell("C2"_pos) == nullptr);
}
