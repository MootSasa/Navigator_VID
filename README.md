# Transport Navigator

Десктопное приложение для построения мультимодальных маршрутов в городе: пешком, на личном авто, такси, автобусе и метро. Алгоритм учитывает текущее время — расписание автобусов, интервалы метро в час пик и днём, разводные мосты ночью. Учебный проект по C++ (2-й семестр, МФТИ).

## Что умеет

- Пять видов передвижения: `walk`, `car`, `taxi`, `bus`, `metro`.
- Три стратегии оптимизации: самый быстрый, самый дешёвый, с минимумом пересадок.
- Динамика рёбер: расписание автобусов, интервал метро (2 мин в часы пик, 5 мин днём), разводные мосты с окном недоступности.
- Безлимитный билет на общественный транспорт: списывается один раз при первой посадке в `bus`/`metro`, дальше Bus/Metro бесплатные.
- Если уже воспользовался общественным транспортом и потом нужно ехать по автомобильному ребру — система автоматически считает это как такси.
- Запрет возврата на личное авто: вышел из машины — обратно не сядешь (но такси всегда доступно).
- Qt6-GUI с интерактивной картой: клик по узлу выбирает старт/финиш, рядом показываются три маршрута на сравнение.

## Алгоритм A*

Поиск пути — A* с эвристикой `евклидово_расстояние / max_speed_strategy` (допустимая, не переоценивает).

Состояние в очереди — не просто узел, а тройка `(node_id, has_public_ticket, has_left_car)`. Эти два флага нужны, потому что одна и та же точка карты может быть достигнута с разной «историей» (с билетом и без), а правила переходов от этой истории зависят.

Стоимость комбинированная — `CombinedCost { time_seconds, money_rub, fatigue, transfers }`. Сравнение стоимостей задаёт стратегия через метод `less()`:

- `FastestStrategy` — сначала время, потом деньги, потом усталость.
- `CheapestStrategy` — сначала деньги (пешком в эту сумму идёт усталость), потом время.
- `MostConvenientStrategy` — сначала число пересадок, потом время.

Правила пересадок:
- `Walk` и `Car` пересадки не дают.
- Внутри общественного транспорта (`Bus`/`Metro`) пересадка — это смена номера маршрута.
- Переход `Taxi → Bus/Metro` или наоборот считается пересадкой.

См. [include/application/a_star_solver.h](include/application/a_star_solver.h), [src/application/a_star_solver.cpp](src/application/a_star_solver.cpp).

## Структура графа

Ориентированный мультиграф: между двумя узлами может быть несколько рёбер разных видов транспорта.

Хранение в `domain::Graph`:
- узлы — `std::unordered_map<NodeId, Node>`,
- список смежности — `std::unordered_map<NodeId, std::vector<Edge>>`.

`Node` содержит ID, имя, координаты в метрах и тип (`Intersection`, `BusStop`, `MetroStation`, `Poi`).

`Edge` помимо ID, концов и длины хранит `std::unique_ptr<ITimeVariant>` — стратегию расчёта времени и стоимости проезда. Конкретные реализации:

- `StaticLogic` — пешком и личным авто: время = длина / скорость.
- `FrequencyBasedLogic` — метро: к движению добавляется ожидание (половина интервала, разного для часа пик и дня).
- `ScheduledLogic` — автобус: ищется ближайшее отправление по списку времён, к движению добавляется ожидание.
- `TimeWindowLogic` — разводной мост: если текущее время попадает в окно `close_hour–open_hour`, ребро возвращает `nullopt` (непроходимо).

Файлы: [include/domain/graph.h](include/domain/graph.h), [include/domain/edge.h](include/domain/edge.h), [include/domain/time_variants.h](include/domain/time_variants.h).

## Архитектура

Код разделён на четыре слоя:

- `domain/` — узлы, рёбра, граф, динамика рёбер, структура стоимости.
- `application/` — алгоритм A*, стратегии маршрутизации, фасад.
- `infrastructure/` — парсер CSV, билдер графа.
- `gui/` — Qt6-виджеты, графические элементы карты.

Из паттернов GoF реально применяются:
- **Strategy** — дважды. `IRouteStrategy` выбирает критерий оптимизации в A*; `ITimeVariant` инкапсулирует поведение конкретного ребра во времени.
- **Facade** — `NavigationFacade` прячет от GUI работу с A*, стратегиями и графом.
- **Builder** — `GraphBuilder` собирает граф из CSV-файлов (узлы → рёбра → параметры → расписания → валидация → готовый объект).

## Формат входных данных

Все данные лежат в `data/` (формат CSV, заголовок в первой строке, строки с `#` — комментарии).

### `nodes.csv` — узлы

| Колонка   | Тип    | Описание                                              |
| --------- | ------ | ----------------------------------------------------- |
| `node_id` | int    | Уникальный ID узла                                    |
| `type`    | string | `intersection` / `bus_stop` / `metro_station` / `poi` |
| `name`    | string | Название (может быть пустым для перекрёстков)         |
| `x`, `y`  | float  | Координаты в метрах                                   |

### `edges.csv` — рёбра

| Колонка                | Тип    | Описание                                  |
| ---------------------- | ------ | ----------------------------------------- |
| `edge_id`              | int    | Уникальный ID ребра                       |
| `from_node`, `to_node` | int    | ID узлов                                  |
| `transport_type`       | string | `walk` / `car` / `bus` / `metro` / `taxi` |
| `length_meters`        | float  | Длина ребра в метрах                      |
| `name`                 | string | Название улицы или номер маршрута         |

Двустороннее движение задаётся двумя рёбрами (A→B и B→A).

### `transport_params.csv` — параметры транспорта

`transport_type, param_name, param_value` — пары имя/значение. Загружаются такие параметры: `speed_kmh`, `fatigue_per_km`, `fuel_consumption_l_per_100km`, `fuel_price_rub_per_l`, `ticket_price`, `price_per_km`, `base_price`, `morning_evening_interval_min`, `day_interval_min`.

### `schedules.csv` — динамика рёбер

| Колонка         | Тип    | Описание                        |
| --------------- | ------ | ------------------------------- |
| `edge_id`       | int    | ID ребра из `edges.csv`         |
| `schedule_type` | string | `fixed_times` или `time_window` |
| `params`        | string | Параметры, зависят от типа      |

- `fixed_times` — список времён отправления через запятую, формат `HH:MM` (например, `"08:00,08:15,08:30"`).
- `time_window` — период недоступности ребра, формат `HH1:MM1-HH2:MM2`. Если первое время больше второго — интервал считается через полночь (например, `23:00-05:00`).

Карту можно править в редакторе из ветки `map-editor`.

## Тесты

Используется GoogleTest 1.14.0 (подтягивается через CMake FetchContent). Все тесты собираются в исполняемый файл `navigator_tests`.

| Файл                                                           | Что проверяет                                                                      |
| -------------------------------------------------------------- | ---------------------------------------------------------------------------------- |
| [test_graph.cpp](tests/test_graph.cpp)                         | Добавление узлов и рёбер, списки смежности                                         |
| [test_csv_parser.cpp](tests/test_csv_parser.cpp)               | Парсинг CSV: кавычки, пустые строки, комментарии, обрезка пробелов                 |
| [test_graph_builder.cpp](tests/test_graph_builder.cpp)         | Сборка графа из CSV, маппинг типов узлов, валидация ссылок                         |
| [test_static_logic.cpp](tests/test_static_logic.cpp)           | Время пешком и на авто = длина / скорость                                          |
| [test_time_variants.cpp](tests/test_time_variants.cpp)         | Расписание автобуса, окно недоступности моста, интервал метро                      |
| [test_strategies.cpp](tests/test_strategies.cpp)               | Стоимости по стратегиям: усталость пешком, топливо у авто, билет у общ. транспорта |
| [test_a_star.cpp](tests/test_a_star.cpp)                       | Выбор быстрого транспорта, недостижимая цель, подсчёт пересадок                    |
| [test_navigation_facade.cpp](tests/test_navigation_facade.cpp) | Поиск маршрутов через фасад всеми тремя стратегиями                                |

## Сборка и запуск

### Требования

- CMake ≥ 3.14
- Компилятор с поддержкой C++17 (GCC, Clang, MSVC)
- Qt6 (только для GUI; ядро и тесты собираются без него)
- Доступ в интернет для FetchContent (один раз, при первой сборке)

### Команды

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# тесты
ctest --test-dir build

# консольный режим (интерактивный REPL)
./build/navigator               # ожидает data/ рядом с местом запуска
./build/navigator path/to/data  # или явный путь к данным

# GUI
./build/navigator_gui
```

Консольный режим принимает строки вида `start_id goal_id [strategy]`, где `strategy` — `fastest`, `cheapest` или `convenient`. Выход — `q`.

## Документация

Doxygen-документация генерируется автоматически из заголовков. Конфигурация — в [Doxyfile](Doxyfile), сборка и публикация на GitHub Pages — в [.github/workflows/docs.yml](.github/workflows/docs.yml) (ветка `gh-pages`).

## Структура репозитория

```
.
├── CMakeLists.txt
├── data/                  # nodes.csv, edges.csv, transport_params.csv, schedules.csv
├── include/
│   ├── domain/            # graph, node, edge, time_variants, transport_params, combined_cost
│   ├── application/       # a_star_solver, navigation_facade, strategies
│   └── infrastructure/    # csv_parser, graph_builder
├── src/                   # реализации по тем же подпапкам + gui/ и main.cpp
├── resources/             # qss-стиль и qrc-ресурсы Qt
└── tests/                 # gtest-тесты по модулям
```

## Авторы

- Шамко Александр
- Чучурюуина Виктория
- Грехова Валентина

## Лицензия

Этот проект распространяется под лицензией **GPL v3.0** — см. [LICENSE](LICENSE).
