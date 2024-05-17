#pragma once

#include <memory>
#include <variant>
#include <vector>

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    void ClearCache();
    bool HasCache() const;

private:
    class Impl {
    public:
        virtual ~Impl() = default;
        [[nodiscard]] virtual CellInterface::Value GetValue() const = 0;
        [[nodiscard]] virtual std::string GetText() const = 0;
        [[nodiscard]] virtual std::vector<Position> GetReferencedCells() const {
            return {};
        }
    };

    class EmptyImpl : public Impl {
    public:
        EmptyImpl() = default;
        [[nodiscard]] CellInterface::Value GetValue() const override {
            return 0.0;
        }
        [[nodiscard]] std::string GetText() const override {
            return {};
        }
    };

    class TextImpl : public Impl {
    public:
        explicit TextImpl(std::string text) : text_{std::move(text)} {}
        [[nodiscard]] CellInterface::Value GetValue() const override {
            return text_.length() > 0 && text_[0] == '\'' ? text_.substr(1) : text_;
        }
        [[nodiscard]] std::string GetText() const override {
            return text_;
        }

    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::string text, SheetInterface& sheet) : formula_{ParseFormula(std::move(text))}, sheet_{sheet} {}
        [[nodiscard]] CellInterface::Value GetValue() const override {
            FormulaInterface::Value val = formula_->Evaluate(sheet_);
            if (std::holds_alternative<double>(val)) {
                return std::get<double>(val);
            }
            return std::get<FormulaError>(val);
        }
        [[nodiscard]] std::string GetText() const override {
            return '=' + formula_->GetExpression();
        }

        [[nodiscard]] std::vector<Position> GetReferencedCells() const override {
            return formula_->GetReferencedCells();
        }

    private:
        std::unique_ptr<FormulaInterface> formula_;
        const SheetInterface& sheet_;
    };

private:
    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;
    mutable std::unique_ptr<Value> cache_;
};
