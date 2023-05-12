#pragma once

#include <algorithm>
#include <variant>

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell();
    ~Cell();

    void Set(std::string text) override;
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

private:
    // можете воспользоваться нашей подсказкой, но это необязательно.
    class Impl {
    public:
        virtual ~Impl() = default;
        virtual CellInterface::Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
    };

    class EmptyImpl : public Impl {
    public:
        EmptyImpl() = default;
        CellInterface::Value GetValue() const override {
            return 0.0;
        }
        std::string GetText() const override {
            return {};
        }
    };

    class TextImpl : public Impl {
    public:
        explicit TextImpl(std::string text) : text_{std::move(text)} {}
        CellInterface::Value GetValue() const override {
            return text_.length() > 0 && text_[0] == '\'' ? text_.substr(1) : text_;
        }
        std::string GetText() const override {
            return text_;
        }

    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        explicit FormulaImpl(std::string text) : formula_{ParseFormula(std::move(text))} {}
        CellInterface::Value GetValue() const override {
            FormulaInterface::Value val = formula_->Evaluate();
            if (std::holds_alternative<double>(val)) {
                return std::get<double>(val);
            }
            return std::get<FormulaError>(val);
        }
        std::string GetText() const override {
            return '=' + formula_->GetExpression();
        }

    private:
        std::unique_ptr<FormulaInterface> formula_;
    };

private:
    std::unique_ptr<Impl> impl_;
};