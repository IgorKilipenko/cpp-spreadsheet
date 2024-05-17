#include "FormulaAST.h"

#include <cassert>
#include <cmath>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>

#include "FormulaBaseListener.h"
#include "FormulaLexer.h"
#include "FormulaParser.h"
#include "common.h"

namespace ASTImpl {

    // Перечисление для обозначения операций
    enum ExpressionPrecedence {
        EP_ADD,
        EP_SUB,
        EP_MUL,
        EP_DIV,
        EP_UNARY,
        EP_ATOM,
        EP_END,
    };

    // Правила приоритетов для расстановки скобок
    enum PrecedenceRule {
        PR_NONE = 0b00,                // never needed
        PR_LEFT = 0b01,                // needed for a left child
        PR_RIGHT = 0b10,               // needed for a right child
        PR_BOTH = PR_LEFT | PR_RIGHT,  // needed for both children
    };

    /**
     * @brief Defines the rules for inserting parentheses based on the precedence of parent and child expressions.
     *
     * The `PRECEDENCE_RULES` table is used to determine if parentheses need to be inserted between a parent
     * and a child of specific precedences. For some nodes, the rules are different for left and right children:
     * (X c Y) p Z  vs  X p (Y c Z).
     *
     * The interesting cases are those where removing the parentheses would change the Abstract Syntax Tree (AST).
     * This can occur when the precedence rules for parentheses differ from the grammatical precedence of operations.
     *
     * Case analysis:
     * - `A + (B + C)` - Always okay. Nothing of lower grammatical precedence could have been written to the right.
     *   - Example: If we had `A + (B + C) / D`, it wouldn't parse in a way that would have given us `A + (B + C)` as a subexpression to deal with.
     * - `A + (B - C)` - Always okay. Nothing of lower grammatical precedence could have been written to the right.
     * - `A - (B + C)` - Never okay.
     * - `A - (B - C)` - Never okay.
     * - `A * (B * C)` - Always okay. The parent has the highest grammatical precedence.
     * - `A * (B / C)` - Always okay. The parent has the highest grammatical precedence.
     * - `A / (B * C)` - Never okay.
     * - `A / (B / C)` - Never okay.
     * - `-(A + B)` - Never okay.
     * - `-(A - B)` - Never okay.
     * - `-(A * B)` - Always okay. The resulting binary operation has the highest grammatical precedence.
     * - `-(A / B)` - Always okay. The resulting binary operation has the highest grammatical precedence.
     * - `+(A + B)` - **Sometimes okay**. For example, parentheses in `+(A + B) / C` are **not** optional.
     *   - Currently in the table, parentheses are always inserted.
     * - `+(A - B)` - **Sometimes okay**. Same as above.
     *   - Currently in the table, parentheses are always inserted.
     * - `+(A * B)` - Always okay. The resulting binary operation has the highest grammatical precedence.
     * - `+(A / B)` - Always okay. The resulting binary operation has the highest grammatical precedence.
     *
     * @note The `PRECEDENCE_RULES` table is a 2D array where `PRECEDENCE_RULES[parent][child]` gives the rule for
     *       whether to insert parentheses between a parent and a child of specific precedences.
     */
    constexpr std::array<std::array<PrecedenceRule, EP_END>, EP_END> PRECEDENCE_RULES = {
        {/* EP_ADD */ {PR_NONE, PR_NONE, PR_NONE, PR_NONE, PR_NONE, PR_NONE},
         /* EP_SUB */ {PR_RIGHT, PR_RIGHT, PR_NONE, PR_NONE, PR_NONE, PR_NONE},
         /* EP_MUL */ {PR_BOTH, PR_BOTH, PR_NONE, PR_NONE, PR_NONE, PR_NONE},
         /* EP_DIV */ {PR_BOTH, PR_BOTH, PR_RIGHT, PR_RIGHT, PR_NONE, PR_NONE},
         /* EP_UNARY */ {PR_BOTH, PR_BOTH, PR_NONE, PR_NONE, PR_NONE, PR_NONE},
         /* EP_ATOM */ {PR_NONE, PR_NONE, PR_NONE, PR_NONE, PR_NONE, PR_NONE}}};

    class Expr {
    public:
        virtual ~Expr() = default;
        virtual void Print(std::ostream& out) const = 0;
        virtual void DoPrintFormula(std::ostream& out, ExpressionPrecedence precedence) const = 0;
        [[nodiscard]] virtual double Evaluate(LookupValue lookup_value) const = 0;

        // higher is tighter
        [[nodiscard]] virtual ExpressionPrecedence GetPrecedence() const = 0;

        void PrintFormula(std::ostream& out, ExpressionPrecedence parent_precedence, bool right_child = false) const {
            auto precedence = GetPrecedence();
            auto mask = right_child ? PR_RIGHT : PR_LEFT;
            bool parens_needed = PRECEDENCE_RULES[parent_precedence][precedence] & mask;
            if (parens_needed) {
                out << '(';
            }

            DoPrintFormula(out, precedence);

            if (parens_needed) {
                out << ')';
            }
        }
    };
}

namespace ASTImpl /* Expr derivatives implementation */ {

    namespace /* BinaryOpExpr implementation */ {
        class BinaryOpExpr final : public Expr {
        public:
            enum Type : char {
                Add = '+',
                Subtract = '-',
                Multiply = '*',
                Divide = '/',
            };

        public:
            explicit BinaryOpExpr(Type type, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
                : type_(type), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

            void Print(std::ostream& out) const override {
                out << '(' << static_cast<char>(type_) << ' ';
                lhs_->Print(out);
                out << ' ';
                rhs_->Print(out);
                out << ')';
            }

            void DoPrintFormula(std::ostream& out, ExpressionPrecedence precedence) const override {
                lhs_->PrintFormula(out, precedence);
                out << static_cast<char>(type_);
                rhs_->PrintFormula(out, precedence, /* right_child = */ true);
            }

            [[nodiscard]] ExpressionPrecedence GetPrecedence() const override {
                switch (type_) {
                case Add:
                    return EP_ADD;
                case Subtract:
                    return EP_SUB;
                case Multiply:
                    return EP_MUL;
                case Divide:
                    return EP_DIV;
                default:
                    // have to do this because VC++ has a buggy warning
                    assert(false);
                    return static_cast<ExpressionPrecedence>(INT_MAX);
                }
            }

            // Метод Evaluate() для бинарных операций.
            // При делении на 0 выбрасывает ошибку вычисления FormulaError
            [[nodiscard]] double Evaluate(LookupValue lookup_value) const override {
                double res;

                switch (type_) {
                case Type::Add: {
                    res = lhs_->Evaluate(lookup_value) + rhs_->Evaluate(lookup_value);
                    break;
                }
                case Type::Subtract: {
                    res = lhs_->Evaluate(lookup_value) - rhs_->Evaluate(lookup_value);
                    break;
                }
                case Type::Multiply: {
                    res = lhs_->Evaluate(lookup_value) * rhs_->Evaluate(lookup_value);
                    break;
                }
                case Type::Divide: {
                    res = lhs_->Evaluate(lookup_value) / rhs_->Evaluate(lookup_value);
                    break;
                }
                default:
                    assert(false);
                }
                if (!std::isfinite(res)) {
                    throw FormulaError(FormulaError::Category::Div0);
                }
                return res;
            }

        private:
            Type type_;
            std::unique_ptr<Expr> lhs_;
            std::unique_ptr<Expr> rhs_;
        };
    }

    namespace /* UnaryOpExpr implementation */ {
        class UnaryOpExpr final : public Expr {
        public:
            enum Type : char {
                UnaryPlus = '+',
                UnaryMinus = '-',
            };

        public:
            explicit UnaryOpExpr(Type type, std::unique_ptr<Expr> operand) : type_(type), operand_(std::move(operand)) {}

            void Print(std::ostream& out) const override {
                out << '(' << static_cast<char>(type_) << ' ';
                operand_->Print(out);
                out << ')';
            }

            void DoPrintFormula(std::ostream& out, ExpressionPrecedence precedence) const override {
                out << static_cast<char>(type_);
                operand_->PrintFormula(out, precedence);
            }

            [[nodiscard]] ExpressionPrecedence GetPrecedence() const override {
                return EP_UNARY;
            }

            // Метод Evaluate() для унарных операций.
            [[nodiscard]] double Evaluate(LookupValue lookup_value) const override {
                switch (type_) {
                case Type::UnaryPlus:
                    return +operand_->Evaluate(lookup_value);
                case Type::UnaryMinus:
                    return -operand_->Evaluate(lookup_value);
                default:
                    assert(false);
                }
            }

        private:
            Type type_;
            std::unique_ptr<Expr> operand_;
        };
    }

    namespace /* NumberExpr implementation */ {
        class NumberExpr final : public Expr {
        public:
            explicit NumberExpr(double value) : value_(value) {}

            void Print(std::ostream& out) const override {
                out << value_;
            }

            void DoPrintFormula(std::ostream& out, ExpressionPrecedence /* precedence */) const override {
                out << value_;
            }

            [[nodiscard]] ExpressionPrecedence GetPrecedence() const override {
                return EP_ATOM;
            }

            // For numbers the method returns the number value.
            [[nodiscard]] double Evaluate(LookupValue /*lookup_value*/) const override {
                return value_;
            }

        private:
            double value_;
        };

    }

    namespace /* CellExpr implementation */ {
        class CellExpr final : public Expr {
        public:
            explicit CellExpr(const Position* cell) : cell_(cell) {}

            void Print(std::ostream& out) const override {
                if (!cell_->IsValid()) {
                    out << FormulaError::Category::Ref;
                } else {
                    out << cell_->ToString();
                }
            }

            void DoPrintFormula(std::ostream& out, ExpressionPrecedence /* precedence */) const override {
                Print(out);
            }

            [[nodiscard]] ExpressionPrecedence GetPrecedence() const override {
                return EP_ATOM;
            }

            double Evaluate(LookupValue lookup_value) const override {
                assert(lookup_value.has_value());
                return lookup_value.value()(*cell_);
            }

        private:
            const Position* cell_;
            std::function<double(Position)> lookup_value_func_;
            std::optional<FormulaError> error_;
        };
    }
}

namespace ASTImpl /* ASTListener implementation */ {

    namespace /* ParseASTListener implementation */ {

        class ParseASTListener final : public FormulaBaseListener {
        public:
            std::unique_ptr<Expr> MoveRoot() {
                assert(args_.size() == 1);
                auto root = std::move(args_.front());
                args_.clear();

                return root;
            }

            std::forward_list<Position> MoveCells() {
                return std::move(cells_);
            }

        public:
            void exitUnaryOp(FormulaParser::UnaryOpContext* ctx) override {
                assert(args_.size() >= 1);

                auto operand = std::move(args_.back());

                UnaryOpExpr::Type type;
                if (ctx->SUB()) {
                    type = UnaryOpExpr::UnaryMinus;
                } else {
                    assert(ctx->ADD() != nullptr);
                    type = UnaryOpExpr::UnaryPlus;
                }

                auto node = std::make_unique<UnaryOpExpr>(type, std::move(operand));
                args_.back() = std::move(node);
            }

            void exitLiteral(FormulaParser::LiteralContext* ctx) override {
                double value = 0;
                auto valueStr = ctx->NUMBER()->getSymbol()->getText();
                std::istringstream in(valueStr);
                in >> value;
                if (!in) {
                    throw ParsingError("Invalid number: " + valueStr);
                }

                auto node = std::make_unique<NumberExpr>(value);
                args_.push_back(std::move(node));
            }

            void exitCell(FormulaParser::CellContext* ctx) override {
                auto value_str = ctx->CELL()->getSymbol()->getText();
                auto value = Position::FromString(value_str);
                if (!value.IsValid()) {
                    throw FormulaException("Invalid position: " + value_str);
                }

                cells_.push_front(value);
                auto node = std::make_unique<CellExpr>(&cells_.front());
                args_.push_back(std::move(node));
            }

            void exitBinaryOp(FormulaParser::BinaryOpContext* ctx) override {
                assert(args_.size() >= 2);

                auto rhs = std::move(args_.back());
                args_.pop_back();

                auto lhs = std::move(args_.back());

                BinaryOpExpr::Type type;
                if (ctx->ADD()) {
                    type = BinaryOpExpr::Add;
                } else if (ctx->SUB()) {
                    type = BinaryOpExpr::Subtract;
                } else if (ctx->MUL()) {
                    type = BinaryOpExpr::Multiply;
                } else {
                    assert(ctx->DIV() != nullptr);
                    type = BinaryOpExpr::Divide;
                }

                auto node = std::make_unique<BinaryOpExpr>(type, std::move(lhs), std::move(rhs));
                args_.back() = std::move(node);
            }

            void visitErrorNode(antlr4::tree::ErrorNode* node) override {
                throw ParsingError("Error when parsing: " + node->getSymbol()->getText());
            }

        private:
            std::vector<std::unique_ptr<Expr>> args_;
            std::forward_list<Position> cells_;
        };
    }

    namespace /* BailErrorListener implementation */ {
        class BailErrorListener : public antlr4::BaseErrorListener {
        public:
            void syntaxError(
                antlr4::Recognizer* /* recognizer */, antlr4::Token* /* offendingSymbol */, size_t /* line */, size_t /* charPositionInLine */,
                const std::string& msg, std::exception_ptr /* e */
                ) override {
                throw ParsingError("Error when lexing: " + msg);
            }
        };
    }
}

FormulaAST ParseFormulaAST(std::istream& in) {
    using namespace antlr4;

    ANTLRInputStream input(in);

    FormulaLexer lexer(&input);
    ASTImpl::BailErrorListener error_listener;
    lexer.removeErrorListeners();
    lexer.addErrorListener(&error_listener);

    CommonTokenStream tokens(&lexer);

    FormulaParser parser(&tokens);
    auto error_handler = std::make_shared<BailErrorStrategy>();
    parser.setErrorHandler(error_handler);
    parser.removeErrorListeners();

    tree::ParseTree* tree = parser.main();
    ASTImpl::ParseASTListener listener;
    tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

    return {listener.MoveRoot(), listener.MoveCells()};
}

FormulaAST ParseFormulaAST(const std::string& in_str) {
    std::istringstream in(in_str);
    return ParseFormulaAST(in);
}

void FormulaAST::PrintCells(std::ostream& out) const {
    for (auto cell : cells_) {
        out << cell.ToString() << ' ';
    }
}

void FormulaAST::Print(std::ostream& out) const {
    root_expr_->Print(out);
}

void FormulaAST::PrintFormula(std::ostream& out) const {
    root_expr_->PrintFormula(out, ASTImpl::EP_ATOM);
}

double FormulaAST::Execute(LookupValue lookup_value) const {
    return root_expr_->Evaluate(lookup_value);
}

FormulaAST::FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr, std::forward_list<Position> cells)
    : root_expr_(std::move(root_expr)), cells_(std::move(cells)) {
    cells_.sort();  // to avoid sorting in GetReferencedCells
}

FormulaAST::~FormulaAST() = default;

const std::forward_list<Position>& FormulaAST::GetCells() const {
    return cells_;
}

std::forward_list<Position>& FormulaAST::GetCells() {
    return cells_;
}
