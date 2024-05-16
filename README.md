# Cpp-Spreadsheet (Электронная таблица)

## Содержание

- [Описание проекта](#project-description)
- [Функциональность](#main-features)
- [Пример использования](#use-examples)
- [Технологии](#project-technology)
- [Установка](#project-installation)

## Описание проекта {#project-description}

Этот проект представляет собой упрощённый аналог электронных таблиц. В ячейках таблицы могут содержаться как текст, так и формулы, включающие индексы (ссылки) других ячеек.

## Функциональность {#main-features}

- Поддержка чисел и строк в ячейках
- Возможность использования формул с числами, строками и ссылками на другие ячейки
- Автоматическое обновление значений ячеек при изменении зависимых ячеек
- Обработка циклических зависимостей и ошибок в формулах

## Пример использования {#use-examples}

```cpp
#include "spreadsheet.h"
#include <iostream>

int main() {
    // Создаем новую таблицу
    auto sheet = CreateSheet();

    // Задаем значения ячеек
    sheet->SetCell("A1"_pos, "Hello, world!");
    sheet->SetCell("B1"_pos, "=2+2");

    // Получаем значения ячеек и выводим на экран
    std::cout << "A1: " << sheet->GetCell("A1"_pos)->GetValue() << std::endl;
    std::cout << "B1: " << sheet->GetCell("B1"_pos)->GetValue() << std::endl;

    // Очищаем ячейку
    sheet->ClearCell("B1"_pos);

    // Получаем значение очищенной ячейки (должно быть пусто)
    std::cout << "B1 after clearing: " << sheet->GetCell("B1"_pos)->GetValue() << std::endl;

    return 0;
}
```

## Технологии {#project-technology}

- Стандарт языка: C++17
- Парсинг формул: [ANTLR4](https://www.antlr.org/)
- Основная логика приложения: библиотека `libspreadsheet`
- Пример использования: консольное приложение `spreadsheet`

## Установка {#project-installation}

<details>

<summary>Для установки проекта выполните следующие шаги:</summary>

```bash
# Клонировать репозиторий
git clone https://github.com/IgorKilipenko/cpp-spreadsheet.git

# Перейти в каталог проекта
cd cpp-spreadsheet

# Собрать проект с помощью CMake
mkdir build
cd build
cmake ..
make

# Установить
make install
```

</details>
