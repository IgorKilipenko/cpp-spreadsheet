#pragma once

#include <iosfwd>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

inline constexpr char FORMULA_SIGN = '=';
inline constexpr char ESCAPE_SIGN = '\'';

/**
 * Position represents a position in a 2D space using row and column indices.
 * Indices are zero-based.
 */
struct Position {
    int row = 0;
    int col = 0;

    bool operator==(Position rhs) const;
    bool operator<(Position rhs) const;

    [[nodiscard]] bool IsValid() const;
    [[nodiscard]] std::string ToString() const;

    static Position FromString(std::string_view str);

    static const int MAX_ROWS = 16384;
    static const int MAX_COLS = 16384;
    static const Position NONE;
};

struct Size {
    int rows = 0;
    int cols = 0;

    bool operator==(Size rhs) const;
};

/**
 * Describes errors that can occur when computing a formula.
 */
class FormulaError {
public:
    enum class Category {
        Ref,            // reference error (cell reference has an invalid position)
        Value,          // the cell value cannot be interpreted as a number
        Div0,           // division by zero occurred during computation
    };

    FormulaError(Category category);

    Category GetCategory() const;

    bool operator==(FormulaError rhs) const;

    std::string_view ToString() const;

private:
    Category category_;
};

std::ostream& operator<<(std::ostream& output, FormulaError fe);

/**
 * Exception thrown when an invalid position is passed to a method
 */
class InvalidPositionException : public std::out_of_range {
public:
    using std::out_of_range::out_of_range;
};

/**
 * FormulaException is an exception that is thrown when a syntactically
 * invalid formula is encountered.
 */
class FormulaException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/**
 * CyclicDependencyException is an exception that is thrown when a circular
 * dependency between cells is detected while computing a formula.
 */
class CircularDependencyException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/**
 * @class CellInterface
 * @brief An interface representing a cell in a spreadsheet.
 *
 * The CellInterface class defines the essential operations that can be performed
 * on a cell within a spreadsheet, such as retrieving its value, text, and any
 * referenced cells. Cells can contain text, numeric values, or formulas that may
 * result in a value or an error.
 */
class CellInterface {
public:
    using Value = std::variant<std::string, double, FormulaError>;

    virtual ~CellInterface() = default;

    /**
     * @brief Returns the visible value of the cell.
     *
     * For a text cell, this returns the text without any escape characters.
     * For a formula cell, this returns the numerical value of the formula or an error message.
     *
     * @return The visible value of the cell as a std::variant containing either a string,
     *         a double, or a FormulaError.
     */
    [[nodiscard]] virtual Value GetValue() const = 0;

    /**
     * @brief Returns the internal text of the cell as if it were being edited.
     *
     * For a text cell, this returns the text, which may include escape characters.
     * For a formula cell, this returns the formula expression.
     *
     * @return The internal text of the cell as a std::string.
     */
    [[nodiscard]] virtual std::string GetText() const = 0;

    /**
     * Returns a list of cells that are directly referenced by this formula.
     * The list is sorted in ascending order and does not contain duplicate cells.
     * In the case of a text cell, the list is empty.
     */
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

// Интерфейс таблицы
class SheetInterface {
public:
    virtual ~SheetInterface() = default;

    /**
     * @brief Sets the content of a cell.
     *
     * If the text begins with the "=" sign, it is interpreted as a formula.
     * If a syntactically incorrect formula is provided, a FormulaException is thrown,
     * and the cell's value remains unchanged.
     * If a formula leads to a cyclic dependency (e.g., if the formula references the
     * current cell), a CircularDependencyException is thrown, and the cell's value
     * remains unchanged.
     *
     * Formula specifics:
     * - If the text contains only the "=" symbol and nothing else, it is not considered a formula.
     * - If the text starts with the "'" (apostrophe) symbol, the apostrophe is omitted when
     *   retrieving the cell's value using the GetValue() method. This can be used if you need
     *   the text to start with the "=" sign without it being interpreted as a formula.
     *
     * @param pos The position of the cell to set.
     * @param text The text to set in the cell.
     */
    virtual void SetCell(Position pos, std::string text) = 0;

    /**
     * @brief Returns the value of the cell at the given position.
     *
     * If the cell is empty, it may return nullptr.
     *
     * @param pos The position of the cell to retrieve.
     * @return A pointer to the cell's interface. If the cell is empty, it may return nullptr.
     */
    [[nodiscard]] virtual const CellInterface* GetCell(Position pos) const = 0;

    /**
     * @brief Retrieves a modifiable pointer to the cell at the given position.
     *
     * This method allows modification of the cell content at the specified position.
     * If the cell is empty, it may return nullptr.
     *
     * @param pos The position of the cell to retrieve.
     * @return A pointer to the cell's interface. If the cell is empty, it may return nullptr.
     */
    virtual CellInterface* GetCell(Position pos) = 0;

    /**
     * @brief Clears the content of a cell.
     *
     * After calling this method, a subsequent call to GetCell() for this cell
     * will return either nullptr or an object with empty text.
     *
     * @param pos The position of the cell to clear.
     */
    virtual void ClearCell(Position pos) = 0;

    /**
     * @brief Computes the size of the area involved in printing.
     *
     * This method determines the bounding rectangle that encompasses all cells
     * with non-empty text.
     *
     * @return The size of the printable area as a Size object.
     */
    [[nodiscard]] virtual Size GetPrintableSize() const = 0;

    /**
     * @brief Outputs the entire sheet to the provided stream.
     *
     * Columns are separated by tab characters, and each row is followed by a newline character.
     * The cells are converted to strings using the GetValue() or GetText() methods, respectively.
     * An empty cell is represented as an empty string in any case.
     *
     * @param output The output stream to which the sheet will be printed.
     */
    virtual void PrintValues(std::ostream& output) const = 0;

    /**
     * @brief Outputs the text content of the entire sheet to the provided stream.
     *
     * Columns are separated by tab characters, and each row is followed by a newline character.
     * The cells are converted to strings using the GetText() method.
     * An empty cell is represented as an empty string.
     *
     * @param output The output stream to which the sheet will be printed.
     */
    virtual void PrintTexts(std::ostream& output) const = 0;
};

// Создаёт готовую к работе пустую таблицу.
std::unique_ptr<SheetInterface> CreateSheet();
