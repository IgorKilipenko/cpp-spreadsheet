#include <iostream>
#include <limits>
#include <variant>

#include "spreadsheet.h"
#include "test_utils/test_runner_p.h"

inline std::ostream& operator<<(std::ostream& output, Position pos) {
    return output << "(" << pos.row << ", " << pos.col << ")";
}

inline Position operator"" _pos(const char* str, std::size_t) {
    return Position::FromString(str);
}

inline std::ostream& operator<<(std::ostream& output, Size size) {
    return output << "(" << size.rows << ", " << size.cols << ")";
}

inline std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
    std::visit(
        [&](const auto& x) {
            output << x;
        },
        value);
    return output;
}

namespace {

    void TestEmpty() {
        auto sheet = CreateSheet();
        ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{0, 0}));
    }

    void TestInvalidPosition() {
        auto sheet = CreateSheet();
        try {
            sheet->SetCell(Position{-1, 0}, "");
        } catch (const InvalidPositionException&) {
        }
        try {
            sheet->GetCell(Position{0, -2});
        } catch (const InvalidPositionException&) {
        }
        try {
            sheet->ClearCell(Position{Position::MAX_ROWS, 0});
        } catch (const InvalidPositionException&) {
        }
    }

    void TestSetCellPlainText() {
        auto sheet = CreateSheet();

        auto checkCell = [&](Position pos, std::string text) {
            sheet->SetCell(pos, text);
            CellInterface* cell = sheet->GetCell(pos);
            ASSERT(cell != nullptr);
            ASSERT_EQUAL(cell->GetText(), text);
            ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), text);
        };

        checkCell("A1"_pos, "Hello");
        checkCell("A1"_pos, "World");
        checkCell("B2"_pos, "Purr");
        checkCell("A3"_pos, "Meow");

        const SheetInterface& constSheet = *sheet;
        ASSERT_EQUAL(constSheet.GetCell("B2"_pos)->GetText(), "Purr");

        sheet->SetCell("A3"_pos, "'=escaped");
        CellInterface* cell = sheet->GetCell("A3"_pos);
        ASSERT_EQUAL(cell->GetText(), "'=escaped");
        ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), "=escaped");
    }

    void TestClearCell() {
        auto sheet = CreateSheet();

        sheet->SetCell("C2"_pos, "Me gusta");
        sheet->ClearCell("C2"_pos);
        ASSERT(sheet->GetCell("C2"_pos) == nullptr);

        sheet->ClearCell("A1"_pos);
        sheet->ClearCell("J10"_pos);
    }
    void TestPrint() {
        auto sheet = CreateSheet();
        sheet->SetCell("A2"_pos, "meow");
        sheet->SetCell("B2"_pos, "=1+2");
        sheet->SetCell("A1"_pos, "=1/0");

        ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{2, 2}));

        std::ostringstream texts;
        sheet->PrintTexts(texts);
        ASSERT_EQUAL(texts.str(), "=1/0\t\nmeow\t=1+2\n");

        std::ostringstream values;
        sheet->PrintValues(values);
        ASSERT_EQUAL(values.str(), "#DIV/0!\t\nmeow\t3\n");

        sheet->ClearCell("B2"_pos);
        ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{2, 1}));
    }
    void TestClearPrint() {
        auto sheet = CreateSheet();
        for (int i = 0; i <= 5; ++i) {
            sheet->SetCell(Position{i, i}, std::to_string(i));
        }

        sheet->ClearCell(Position{3, 3});

        std::ostringstream sizes;
        for (int i = 5; i >= 0; --i) {
            sheet->ClearCell(Position{i, i});
            sizes << sheet->GetPrintableSize();
        }
        ASSERT_EQUAL(sizes.str(), "(5, 5)(3, 3)(3, 3)(2, 2)(1, 1)(0, 0)");
    }
}  // namespace

namespace tests {

    void TestPositionAndStringConversion() {
        auto testSingle = [](Position pos, std::string_view str) {
            ASSERT_EQUAL(pos.ToString(), str);
            ASSERT_EQUAL(Position::FromString(str), pos);
        };

        for (int i = 0; i < 25; ++i) {
            testSingle(Position{i, i}, char('A' + i) + std::to_string(i + 1));
        }

        testSingle(Position{0, 0}, "A1");
        testSingle(Position{0, 1}, "B1");
        testSingle(Position{0, 25}, "Z1");
        testSingle(Position{0, 26}, "AA1");
        testSingle(Position{0, 27}, "AB1");
        testSingle(Position{0, 51}, "AZ1");
        testSingle(Position{0, 52}, "BA1");
        testSingle(Position{0, 53}, "BB1");
        testSingle(Position{0, 77}, "BZ1");
        testSingle(Position{0, 78}, "CA1");
        testSingle(Position{0, 701}, "ZZ1");
        testSingle(Position{0, 702}, "AAA1");
        testSingle(Position{136, 2}, "C137");
        testSingle(Position{Position::MAX_ROWS - 1, Position::MAX_COLS - 1}, "XFD16384");
    }

    void TestPositionToStringInvalid() {
        ASSERT_EQUAL((Position{-1, -1}).ToString(), "");
        ASSERT_EQUAL((Position{-10, 0}).ToString(), "");
        ASSERT_EQUAL((Position{1, -3}).ToString(), "");
    }

    void TestStringToPositionInvalid() {
        ASSERT(!Position::FromString("").IsValid());
        ASSERT(!Position::FromString("A").IsValid());
        ASSERT(!Position::FromString("1").IsValid());
        ASSERT(!Position::FromString("e2").IsValid());
        ASSERT(!Position::FromString("A0").IsValid());
        ASSERT(!Position::FromString("A-1").IsValid());
        ASSERT(!Position::FromString("A+1").IsValid());
        ASSERT(!Position::FromString("R2D2").IsValid());
        ASSERT(!Position::FromString("C3PO").IsValid());
        ASSERT(!Position::FromString("XFD16385").IsValid());
        ASSERT(!Position::FromString("XFE16384").IsValid());
        ASSERT(!Position::FromString("A1234567890123456789").IsValid());
        ASSERT(!Position::FromString("ABCDEFGHIJKLMNOPQRS8").IsValid());
    }

    void TestEmpty() {
        auto sheet = CreateSheet();
        ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{0, 0}));
    }

    void TestInvalidPosition() {
        auto sheet = CreateSheet();
        try {
            sheet->SetCell(Position{-1, 0}, "");
        } catch (const InvalidPositionException&) {
        }
        try {
            sheet->GetCell(Position{0, -2});
        } catch (const InvalidPositionException&) {
        }
        try {
            sheet->ClearCell(Position{Position::MAX_ROWS, 0});
        } catch (const InvalidPositionException&) {
        }
    }

    void TestSetCellPlainText() {
        auto sheet = CreateSheet();

        auto checkCell = [&](Position pos, std::string text) {
            sheet->SetCell(pos, text);
            CellInterface* cell = sheet->GetCell(pos);
            ASSERT(cell != nullptr);
            ASSERT_EQUAL(cell->GetText(), text);
            ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), text);
        };

        checkCell("A1"_pos, "Hello");
        checkCell("A1"_pos, "World");
        checkCell("B2"_pos, "Purr");
        checkCell("A3"_pos, "Meow");

        const SheetInterface& constSheet = *sheet;
        ASSERT_EQUAL(constSheet.GetCell("B2"_pos)->GetText(), "Purr");

        sheet->SetCell("A3"_pos, "'=escaped");
        CellInterface* cell = sheet->GetCell("A3"_pos);
        ASSERT_EQUAL(cell->GetText(), "'=escaped");
        ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), "=escaped");
    }

    void TestClearCell() {
        auto sheet = CreateSheet();

        sheet->SetCell("C2"_pos, "Me gusta");
        sheet->ClearCell("C2"_pos);
        ASSERT(sheet->GetCell("C2"_pos) == nullptr);

        sheet->ClearCell("A1"_pos);
        sheet->ClearCell("J10"_pos);
    }

    void TestFormulaArithmetic() {
        auto sheet = CreateSheet();
        auto evaluate = [&](std::string expr) {
            return std::get<double>(ParseFormula(std::move(expr))->Evaluate(*sheet));
        };

        ASSERT_EQUAL(evaluate("1"), 1);
        ASSERT_EQUAL(evaluate("42"), 42);
        ASSERT_EQUAL(evaluate("2 + 2"), 4);
        ASSERT_EQUAL(evaluate("2 + 2*2"), 6);
        ASSERT_EQUAL(evaluate("4/2 + 6/3"), 4);
        ASSERT_EQUAL(evaluate("(2+3)*4 + (3-4)*5"), 15);
        ASSERT_EQUAL(evaluate("(12+13) * (14+(13-24/(1+1))*55-46)"), 575);
    }

    void TestFormulaReferences() {
        auto sheet = CreateSheet();
        auto evaluate = [&](std::string expr) {
            return std::get<double>(ParseFormula(std::move(expr))->Evaluate(*sheet));
        };

        sheet->SetCell("A1"_pos, "1");
        ASSERT_EQUAL(evaluate("A1"), 1);
        sheet->SetCell("A2"_pos, "2");
        ASSERT_EQUAL(evaluate("A1+A2"), 3);

        // Тест на нули:
        sheet->SetCell("B3"_pos, "");
        ASSERT_EQUAL(evaluate("A1+B3"), 1);  // Ячейка с пустым текстом
        ASSERT_EQUAL(evaluate("A1+B1"), 1);  // Пустая ячейка
        ASSERT_EQUAL(evaluate("A1+E4"), 1);  // Ячейка за пределами таблицы
    }

    void TestFormulaExpressionFormatting() {
        auto reformat = [](std::string expr) {
            return ParseFormula(std::move(expr))->GetExpression();
        };

        ASSERT_EQUAL(reformat("  1  "), "1");
        ASSERT_EQUAL(reformat("  -1  "), "-1");
        ASSERT_EQUAL(reformat("2 + 2"), "2+2");
        ASSERT_EQUAL(reformat("(2*3)+4"), "2*3+4");
        ASSERT_EQUAL(reformat("(2*3)-4"), "2*3-4");
        ASSERT_EQUAL(reformat("( ( (  1) ) )"), "1");
    }

    void TestFormulaReferencedCells() {
        ASSERT(ParseFormula("1")->GetReferencedCells().empty());

        auto a1 = ParseFormula("A1");
        ASSERT_EQUAL(a1->GetReferencedCells(), (std::vector{"A1"_pos}));

        auto b2c3 = ParseFormula("B2+C3");
        ASSERT_EQUAL(b2c3->GetReferencedCells(), (std::vector{"B2"_pos, "C3"_pos}));

        auto tricky = ParseFormula("A1 + A2 + A1 + A3 + A1 + A2 + A1");
        ASSERT_EQUAL(tricky->GetExpression(), "A1+A2+A1+A3+A1+A2+A1");
        ASSERT_EQUAL(tricky->GetReferencedCells(), (std::vector{"A1"_pos, "A2"_pos, "A3"_pos}));
    }

    void TestErrorValue() {
        auto sheet = CreateSheet();
        sheet->SetCell("E2"_pos, "A1");
        sheet->SetCell("E4"_pos, "=E2");
        ASSERT_EQUAL(sheet->GetCell("E4"_pos)->GetValue(), CellInterface::Value(FormulaError::Category::Value));

        sheet->SetCell("E2"_pos, "3D");
        ASSERT_EQUAL(sheet->GetCell("E4"_pos)->GetValue(), CellInterface::Value(FormulaError::Category::Value));
    }

    void TestErrorDiv0() {
        auto sheet = CreateSheet();

        constexpr double max = std::numeric_limits<double>::max();

        sheet->SetCell("A1"_pos, "=1/0");
        ASSERT_EQUAL(sheet->GetCell("A1"_pos)->GetValue(), CellInterface::Value(FormulaError::Category::Div0));

        sheet->SetCell("A1"_pos, "=1e+200/1e-200");
        ASSERT_EQUAL(sheet->GetCell("A1"_pos)->GetValue(), CellInterface::Value(FormulaError::Category::Div0));

        sheet->SetCell("A1"_pos, "=0/0");
        ASSERT_EQUAL(sheet->GetCell("A1"_pos)->GetValue(), CellInterface::Value(FormulaError::Category::Div0));

        {
            std::ostringstream formula;
            formula << '=' << max << '+' << max;
            sheet->SetCell("A1"_pos, formula.str());
            ASSERT_EQUAL(sheet->GetCell("A1"_pos)->GetValue(), CellInterface::Value(FormulaError::Category::Div0));
        }

        {
            std::ostringstream formula;
            formula << '=' << -max << '-' << max;
            sheet->SetCell("A1"_pos, formula.str());
            ASSERT_EQUAL(sheet->GetCell("A1"_pos)->GetValue(), CellInterface::Value(FormulaError::Category::Div0));
        }

        {
            std::ostringstream formula;
            formula << '=' << max << '*' << max;
            sheet->SetCell("A1"_pos, formula.str());
            ASSERT_EQUAL(sheet->GetCell("A1"_pos)->GetValue(), CellInterface::Value(FormulaError::Category::Div0));
        }
    }

    void TestEmptyCellTreatedAsZero() {
        auto sheet = CreateSheet();
        sheet->SetCell("A1"_pos, "=B2");
        ASSERT_EQUAL(sheet->GetCell("A1"_pos)->GetValue(), CellInterface::Value(0.0));
    }

    void TestFormulaInvalidPosition() {
        auto sheet = CreateSheet();
        auto try_formula = [&](const std::string& formula) {
            try {
                sheet->SetCell("A1"_pos, formula);
                ASSERT(false);
            } catch (const FormulaException&) {
                // we expect this one
            }
        };

        try_formula("=X0");
        try_formula("=ABCD1");
        try_formula("=A123456");
        try_formula("=ABCDEFGHIJKLMNOPQRS1234567890");
        try_formula("=XFD16385");
        try_formula("=XFE16384");
        try_formula("=R2D2");
    }

    void TestPrint() {
        auto sheet = CreateSheet();
        sheet->SetCell("A2"_pos, "meow");
        sheet->SetCell("B2"_pos, "=35");

        ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{2, 2}));

        std::ostringstream texts;
        sheet->PrintTexts(texts);
        ASSERT_EQUAL(texts.str(), "\t\nmeow\t=35\n");

        std::ostringstream values;
        sheet->PrintValues(values);
        ASSERT_EQUAL(values.str(), "\t\nmeow\t35\n");
    }

    void TestCellReferences() {
        auto sheet = CreateSheet();
        sheet->SetCell("A1"_pos, "1");
        sheet->SetCell("A2"_pos, "=A1");
        sheet->SetCell("B2"_pos, "=A1");

        ASSERT(sheet->GetCell("A1"_pos)->GetReferencedCells().empty());
        ASSERT_EQUAL(sheet->GetCell("A2"_pos)->GetReferencedCells(), std::vector{"A1"_pos});
        ASSERT_EQUAL(sheet->GetCell("B2"_pos)->GetReferencedCells(), std::vector{"A1"_pos});

        // Ссылка на пустую ячейку
        sheet->SetCell("B2"_pos, "=B1");
        ASSERT(sheet->GetCell("B1"_pos)->GetReferencedCells().empty());
        ASSERT_EQUAL(sheet->GetCell("B2"_pos)->GetReferencedCells(), std::vector{"B1"_pos});

        sheet->SetCell("A2"_pos, "");
        ASSERT(sheet->GetCell("A1"_pos)->GetReferencedCells().empty());
        ASSERT(sheet->GetCell("A2"_pos)->GetReferencedCells().empty());

        // Ссылка на ячейку за пределами таблицы
        sheet->SetCell("B1"_pos, "=C3");
        ASSERT_EQUAL(sheet->GetCell("B1"_pos)->GetReferencedCells(), std::vector{"C3"_pos});
    }

    void TestGraph() {
        spreadsheet::Sheet raw_sheet;
        raw_sheet.SetCell("A1"_pos, "=A2+A3+A4+A5");
        const graph::DependencyGraph& graph = raw_sheet.GetGraph();
        ASSERT_EQUAL(graph.GetEdgeCount(), 4);
        ASSERT_EQUAL(graph.GetVertexCount(), 1);

        raw_sheet.SetCell("A1"_pos, "=A2+A3+A4");
        ASSERT_EQUAL(graph.GetEdgeCount(), 3);
        ASSERT_EQUAL(graph.GetVertexCount(), 1);

        raw_sheet.SetCell("B1"_pos, "=A4");
        ASSERT_EQUAL(graph.GetEdgeCount(), 4);
        ASSERT_EQUAL(graph.GetVertexCount(), 2);

        raw_sheet.SetCell("B2"_pos, "=A1");
        ASSERT_EQUAL(graph.GetEdgeCount(), 5);
        ASSERT_EQUAL(graph.GetVertexCount(), 3);

        raw_sheet.SetCell("A1"_pos, "=A2+A3+A4+A5");
        ASSERT_EQUAL(graph.GetEdgeCount(), 6);
        ASSERT_EQUAL(graph.GetVertexCount(), 3);
    }

    void TestFormulaIncorrect() {
        auto isIncorrect = [](std::string expression) {
            try {
                ParseFormula(std::move(expression));
            } catch (const FormulaException&) {
                return true;
            }
            return false;
        };

        ASSERT(isIncorrect("A2B"));
        ASSERT(isIncorrect("3X"));
        ASSERT(isIncorrect("A0++"));
        ASSERT(isIncorrect("((1)"));
        ASSERT(isIncorrect("2+4-"));
    }

    void TestCellCircularReferences() {
        {
            auto sheet = CreateSheet();
            sheet->SetCell("E2"_pos, "=E4");
            sheet->SetCell("E4"_pos, "=X9");
            sheet->SetCell("X9"_pos, "=M6");
            sheet->SetCell("M6"_pos, "Ready");

            bool caught = false;
            try {
                sheet->SetCell("M6"_pos, "=E2");
            } catch (const CircularDependencyException&) {
                caught = true;
            }

            ASSERT(caught);
            ASSERT_EQUAL(sheet->GetCell("M6"_pos)->GetText(), "Ready");
        }
        {
            auto sheet = CreateSheet();

            bool caught = false;
            try {
                sheet->SetCell("A1"_pos, "=A1");
            } catch (const CircularDependencyException&) {
                caught = true;
            }
            ASSERT(caught);
        }
        {
            auto sheet = CreateSheet();
            sheet->SetCell("A1"_pos, "=A2");

            bool caught = false;
            try {
                sheet->SetCell("A2"_pos, "=A1");
            } catch (const CircularDependencyException&) {
                caught = true;
            }
            ASSERT(caught);

            sheet->SetCell("A2"_pos, "=A3");
            caught = false;
            try {
                sheet->SetCell("A3"_pos, "=A1");
            } catch (const CircularDependencyException&) {
                caught = true;
            }
            ASSERT(caught);
        }
        {
            auto sheet = CreateSheet();
            sheet->SetCell("A1"_pos, "=A2+A3");
            sheet->SetCell("A2"_pos, "=C1+C2");
            sheet->SetCell("A5"_pos, "=C1+C2");
            sheet->SetCell("B1"_pos, "=A2+A3+B3");
            sheet->SetCell("B2"_pos, "=A2+A3+B3");
            sheet->SetCell("B10"_pos, "=B2+A1");

            bool caught = false;
            try {
                sheet->SetCell("A2"_pos, "=B1");
            } catch (const CircularDependencyException&) {
                caught = true;
            }
            ASSERT(caught);
        }
    }

    void TestSetPrint() {
        auto sheet = CreateSheet();
        std::ostringstream sizes;
        for (int i = 0; i <= 5; ++i) {
            sheet->SetCell(Position{i, i}, std::to_string(i));
            sizes << sheet->GetPrintableSize();
        }

        ASSERT_EQUAL(sizes.str(), "(1, 1)(2, 2)(3, 3)(4, 4)(5, 5)(6, 6)");
    }

    void TestInvalidateCache() {
        {
            spreadsheet::Sheet sheet;
            const auto pos = "A1"_pos;
            sheet.SetCell("A2"_pos, "5");
            sheet.SetCell("A1"_pos, "=A2+A3");
            Cell* cell = sheet.GetCell(pos);
            ASSERT(std::holds_alternative<double>(cell->GetValue()));
            ASSERT_EQUAL(std::get<double>(cell->GetValue()), 5);
            ASSERT(cell->HasCache());

            sheet.SetCell("A5"_pos, "");
            ASSERT(cell->HasCache());

            sheet.SetCell("A3"_pos, "0");
            ASSERT(!cell->HasCache());
            ASSERT(sheet.GetCell("A2"_pos)->HasCache());

            cell->GetValue();
            sheet.SetCell("A3"_pos, "0");
            ASSERT(cell->HasCache());

            cell->ClearCache();
            ASSERT(!cell->HasCache());
            ASSERT_EQUAL(std::get<double>(cell->GetValue()), 5);
            ASSERT(cell->HasCache());

            cell->Clear();
            ASSERT(!cell->HasCache());
        }
        {
            spreadsheet::Sheet sheet;
            const auto pos = "A1"_pos;
            sheet.SetCell("A2"_pos, "5");
            sheet.SetCell("A1"_pos, "=A2+A3");

            Cell* cell = sheet.GetCell(pos);
            ASSERT(std::holds_alternative<double>(cell->GetValue()));
            ASSERT_EQUAL(std::get<double>(cell->GetValue()), 5);
            ASSERT(cell->HasCache());

            sheet.ClearCell("A2"_pos);
            ASSERT(!cell->HasCache());
        }
    }
}  // namespace

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestEmpty);
    RUN_TEST(tr, TestInvalidPosition);
    RUN_TEST(tr, TestSetCellPlainText);
    RUN_TEST(tr, TestClearCell);
    RUN_TEST(tr, TestPrint);
    RUN_TEST(tr, TestClearPrint);

    RUN_TEST(tr, tests::TestPositionAndStringConversion);
    RUN_TEST(tr, tests::TestPositionToStringInvalid);
    RUN_TEST(tr, tests::TestStringToPositionInvalid);
    RUN_TEST(tr, tests::TestEmpty);
    RUN_TEST(tr, tests::TestInvalidPosition);
    RUN_TEST(tr, tests::TestSetCellPlainText);
    RUN_TEST(tr, tests::TestClearCell);
    RUN_TEST(tr, tests::TestFormulaArithmetic);
    RUN_TEST(tr, tests::TestFormulaReferences);
    RUN_TEST(tr, tests::TestFormulaExpressionFormatting);
    RUN_TEST(tr, tests::TestFormulaReferencedCells);
    RUN_TEST(tr, tests::TestErrorValue);
    RUN_TEST(tr, tests::TestErrorDiv0);
    RUN_TEST(tr, tests::TestEmptyCellTreatedAsZero);
    RUN_TEST(tr, tests::TestFormulaInvalidPosition);
    RUN_TEST(tr, tests::TestPrint);
    RUN_TEST(tr, tests::TestCellReferences);
    RUN_TEST(tr, tests::TestFormulaIncorrect);
    RUN_TEST(tr, tests::TestCellCircularReferences);
    RUN_TEST(tr, tests::TestGraph);
    RUN_TEST(tr, tests::TestSetPrint);
    RUN_TEST(tr, tests::TestInvalidateCache);

    return 0;
}
