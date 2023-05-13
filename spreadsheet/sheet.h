#pragma once

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include "cell.h"
#include "common.h"

namespace /* Sheet definations */ {
    class Sheet : public SheetInterface {
    private:
        using Column = std::unordered_map<int, std::unique_ptr<Cell>>;

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
        std::unordered_map<int, Column> sheet_;
        Size size_ = {0, 0};

        void ValidatePosition_(const Position& pos) const {
            if (!pos.IsValid()) {
                throw InvalidPositionException("Invalid cell position");
            }
        }

        template <typename TPosition, std::enable_if_t<std::is_same_v<std::decay_t<TPosition>, Position>, bool> = true>
        const Cell* GetCell_(TPosition&& pos) const;
        Size CalculateSize_() const;
        void PrintValue_(std::ostream& output, const Cell::Value& value) const;
        bool IsLastPosition_(const Position& pos) const {
            return size_.rows - pos.row == 1 || size_.cols - pos.col == 1;
        }
    };
}

namespace /* Sheet template implementation */ {
    template <typename TPosition, std::enable_if_t<std::is_same_v<std::decay_t<TPosition>, Position>, bool>>
    const Cell* Sheet::GetCell_(TPosition&& pos) const {
        if (const auto row_ptr = sheet_.find(pos.row); row_ptr != sheet_.end()) {
            if (const auto cell_ptr = row_ptr->second.find(pos.col); cell_ptr != row_ptr->second.end()) {
                return cell_ptr->second.get();
            }
        }
        return nullptr;
    }
}