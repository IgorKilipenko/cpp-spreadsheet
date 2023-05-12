#include "formula.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

#include "FormulaAST.h"

using namespace std::literals;

std::ostream &operator<<(std::ostream &output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
    class Formula : public FormulaInterface {
    public:
        // Реализуйте следующие методы:
        //! explicit Formula(std::string expression)
        //! Value Evaluate() const override
        //! std::string GetExpression() const override
        explicit Formula(std::string expression) : ast_(ParseFormulaAST(expression)){};

        Value Evaluate() const override {
            Value res;
            try {
                res = ast_.Execute();
            } catch (const FormulaError &err) {
                res = err;
            }
            return res;
        }

        std::string GetExpression() const override {
            std::ostringstream out;
            ast_.PrintFormula(out);
            return out.str();
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