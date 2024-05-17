#include "formula.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>

#include "FormulaAST.h"

using namespace std::literals;

std::ostream &operator<<(std::ostream &output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace formula::helpers {
    void MakeUnique(std::vector<Position> &positions) {
        std::sort(positions.begin(), positions.end());
        auto last = std::unique(positions.begin(), positions.end());
        positions.erase(last, positions.end());
    }
}

namespace {
    class Formula : public FormulaInterface {
    public:
        explicit Formula(std::string expression) : ast_(ParseFormulaAST(expression)){};

        [[nodiscard]] Value Evaluate(const SheetInterface &sheet) const override {
            const auto lookup_value = [&sheet](const Position &position) -> double {
                const CellInterface *cell_ptr = sheet.GetCell(position);
                if (cell_ptr == nullptr) {
                    return 0.0;
                }

                CellInterface::Value cell_value = cell_ptr->GetValue();

                if (const FormulaError *error = std::get_if<FormulaError>(&cell_value); error != nullptr) {
                    throw *error;
                }

                if (const double *result = std::get_if<double>(&cell_value); result != nullptr) {
                    return *result;
                }

                if (const std::string *str = std::get_if<std::string>(&cell_value); str != nullptr) {
                    size_t idx = 0;
                    try {
                        double result = std::stod(*str, &idx);
                        if (idx < str->size()) {
                            throw FormulaError(FormulaError::Category::Value);
                        }
                        return result;
                    } catch (...) {
                    }
                }

                throw FormulaError(FormulaError::Category::Value);
            };

            Value res;
            try {
                res = ast_.Execute(lookup_value);
            } catch (const FormulaError &err) {
                res = err;
            }
            return res;
        }

        [[nodiscard]] std::string GetExpression() const override {
            std::ostringstream out;
            ast_.PrintFormula(out);
            return out.str();
        }

        [[nodiscard]] std::vector<Position> GetReferencedCells() const override {
            const auto &cell_refs = ast_.GetCells();
            auto result = std::vector<Position>(cell_refs.begin(), cell_refs.end());
            formula::helpers::MakeUnique(result);
            return result;
        }

    private:
        FormulaAST ast_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(expression);
    } catch (...) {
        throw FormulaException("Parsing formula from expression was failure"s);
    }
}

FormulaError::FormulaError(Category category) : category_(category) {}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return ToString() == rhs.ToString();
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
    case Category::Ref:
        return "#REF!"sv;
    case Category::Value:
        return "#VALUE!"sv;
    case Category::Div0:
        return "#DIV/0!"sv;
    default:
        assert(false);
        return ""sv;
    }
}
