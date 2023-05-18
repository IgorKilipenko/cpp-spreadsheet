#pragma once

#include <forward_list>
#include <functional>
#include <optional>
#include <stdexcept>

#include "FormulaLexer.h"
#include "common.h"

namespace ASTImpl {
    class Expr;
}

class ParsingError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

using LookupValue = std::optional<std::function<double(const Position&)>>;

class FormulaAST {

public:
    FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr, std::forward_list<Position> cells);
    FormulaAST(FormulaAST&&) = default;
    FormulaAST& operator=(FormulaAST&&) = default;
    ~FormulaAST();

    double Execute(LookupValue lookup_value) const;
    void Print(std::ostream& out) const;
    void PrintFormula(std::ostream& out) const;
    void PrintCells(std::ostream& out) const;
    std::forward_list<Position>& GetCells();
    const std::forward_list<Position>& GetCells() const;

private:
    std::unique_ptr<ASTImpl::Expr> root_expr_;
    std::forward_list<Position> cells_;
};

FormulaAST ParseFormulaAST(std::istream& in);
FormulaAST ParseFormulaAST(const std::string& in_str);