#pragma once

#include <memory>
#include <vector>

#include "common.h"

/**
 * @brief Interface for working with formulas.
 *
 * This interface provides the ability to evaluate and update arithmetic expressions,
 * which may include simple binary operations, numbers, parentheses, and cell references.
 * Cells referenced in the formula can contain either formulas or text. If a cell contains
 * text that represents a number, it will be treated as a number. An empty cell
 * or a cell with an empty text is treated as the number zero.
 */
class FormulaInterface {
public:
    using Value = std::variant<double, FormulaError>;

    virtual ~FormulaInterface() = default;

    /**
     * @brief Evaluates the formula and returns its computed value or an error.
     *
     * @param sheet The sheet interface containing the cell values referenced by the formula.
     * @return The computed value of the formula if all referenced cells are valid, or an error
     *         if any referenced cell evaluation results in an error. If multiple errors occur,
     *         any one of them may be returned.
     */
    [[nodiscard]] virtual Value Evaluate(const SheetInterface& sheet) const = 0;

    /**
     * @brief Returns the expression that describes the formula.
     *
     * The returned expression does not contain any whitespace or unnecessary parentheses.
     *
     * @return The expression representing the formula.
     */
    [[nodiscard]] virtual std::string GetExpression() const = 0;

    /**
     * @brief Returns a list of cells that are directly involved in the evaluation of the formula.
     *
     * The list is sorted in ascending order and does not contain duplicate cells.
     *
     * @return A vector of Position objects representing the cells referenced by the formula.
     */
    [[nodiscard]] virtual std::vector<Position> GetReferencedCells() const = 0;
};

/**
 * @brief Parses the given expression and returns a unique_ptr to a FormulaInterface object.
 *
 * @throws FormulaException if the formula is syntactically incorrect.
 *
 * @param expression The string representation of the formula to parse.
 * @return A unique_ptr to a FormulaInterface object representing the parsed formula.
 */
std::unique_ptr<FormulaInterface> ParseFormula(std::string expression);
