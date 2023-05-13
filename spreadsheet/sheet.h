#pragma once

#include <functional>

#include "cell.h"
#include "common.h"

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    // Можете дополнить ваш класс нужными полями и методами
    struct Hasher {
    public:
        size_t operator()(const Position& pos) const {
            return pos.row + pos.col * INDEX;
        }

    private:
        static const size_t INDEX = 42;
    };

private:
    // Можете дополнить ваш класс нужными полями и методами
    std::unordered_map<Position, std::unique_ptr<Cell>, Hasher> sheet_;
    Size size_ = {0, 0};

    void ValidatePosition_(const Position& pos) const {
        if (!pos.IsValid()) {
            throw InvalidPositionException("Position is invalid");
        }
    }

    const CellInterface* GetCell_(Position&& pos) const;
    Size CalculateSize_() const;
    void PrintValue_(std::ostream& output, const Cell::Value& value) const;
};