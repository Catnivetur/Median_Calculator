# Median Calculator

## Описание приложения

Программа выполняет расчёт различных метрик на основе цены на бирже. Данные считываются из входных файлов, обрабатываются и записываются в соответсвующие выходные файлы в указаной директории.
Поддерживается вычисление следующих метрик:
- Медиана (price_median)
- Среднее значение (price_mean)
- Стандартное отклонение (price_standart_deviation)
- Квантили (price_percentiles): p50, p90, p95, p99

Метрики рассчитываются постепенно и записываются только в том случае, если их значение поменялось.
Обрабатываются только те `.csv` файлы, которые содержат поля `receive_ts` и `price`. 

## Требования

Программа написана с использованием стандарта C++ 23.
Протестированные платформы: Arch Linux и Windows 11.
Протестированные компиляторы:
- clang, GCC 15.2 (Arch Linux);
- clang, MSVC (Windows 11).

### Зависимости

Программа использует следующие сторонние зависимости:
- boost https://github.com/boostorg/boost.git
- spdlog https://github.com/gabime/spdlog.git
- tom++ https://github.com/marzer/tomlplusplus.git

Системы сборки: Ninja или Unix Makefiles (только для Linux).

## Сборка проекта

Запуск из директории с программой.
Для сборки рекомендуется использование компилятора clang. Указать нужный компилятор можно добавив параметр `-DCMAKE_C_COMPILER={компилятор C}` и `-DCMAKE_CXX_COMPILER={компилятор C++}` в строку как аргументы.
Сборка с использованием Ninja:
```bash
cmake --preset default-ninja
cmake --build --preset default-ninja
```
Сборка с использованием Unix Makefiles (Linux only):
```bash
cmake --preset default-makefiles
cmake --build --preset default-makefiles
```

Сборка с использованием MSVC (Windows only):
```bash
cmake --preset msvc
cmake --build --preset msvc --config Release
```

Пример с явным указанием компилятора clang для Windows:
```bash
cmake --preset default-ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl
cmake --build --preset default-ninja
```
Для Linux:
```bash
cmake --preset default-ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build --preset default-ninja
```

## Запуск приложения

Программа имеет два параметра запуска:
1. `--config`/`--cfg` - указывает путь к конфигурационному TOML-файлу.
Если параметр не указан программа загрузит стандартный файл `config.toml`, находящийся в корневой папке.
2. `--parallel`/`--p` - включает режим параллельной обработки. Целесообразно использовать для большого объёма данных.
Если параметр не указан, программа работает в обычном (последовательном) режиме.

Оба параметра могут передававться как с двойным, так и с одинарным тире.

### Примеры использования

Самый простой вариант:
```bash
    .\build\Median_Calculator.exe
```
Запускает программу используя стандартный конфигурационный файл `config.toml` из коренной папки в обычном (последовательном) режиме.

```bash
    .\build\Median_Calculator.exe --config prod.toml -parallel
```
или
```bash
    .\build\Median_Calculator.exe -cfg prod.toml --p
```
Указывает prod.toml как файл конфигурации и запускает программу в режиме параллельной обработки.

## Формат конфигурации

Файл конфигурации имеет следующий вид:
```toml
[main]
    input = './examples/input'
    output = './examples/output'
    filename_mask = ['']
[main.metrics]
    price_median = true
    price_mean = true
    price_standart_deviation = true
    price_percentiles = true
```
Так выглядит стандартный файл конфигурации.

Параметры `[main]`:
- input - путь к папке с `.csv` файлами.
- output - путь к папке с выходными файлами. Если не указан, создается директория `output` в текущей рабочей директории. Путь: `./examples/output`.
- filename_mask - список строк для фильтрации имен файлов. Читаются только те файлы, имена которых содержат хотя бы одну из указанных масок. Если список пустой, читаются все CSV-файлы.

Параметры `[mainmain.metrics]` указывают на то, какие расчёты проводить:
- price_median - расчёт медианы.
- price_mean - расчёт среднего значения.
- price_standart_deviation - расчёт стандартного отклонения.
- price_percentiles - расчёт квантилий p50, p90, p95, p99.
Если никакие метрики не указаны, программа прекращает свою работу без рассчётов.

## Примеры

### Запуск

На примере стандартного файла конфигурации (без выбора аргументов):
```bash
    .\build\Median_Calculator.exe
```

### Входные данные

Файл `data_1.csv` имеет вид из папки с входными данными:
```csv
receive_ts;exchange_ts;price;quantity;side;rebuild
1770656944933436;1770656944930164;913725.38299291;15.43080351;bid;1
1770656944930828;1770656944930202;813416.21882447;4.97446860;ask;0
1770656944932363;1770656944930210;768760.11567887;13.08232216;ask;1
```
Файл `data_2.csv` имеет вид из папки с входными данными:
```csv
receive_ts;exchange_ts;price;quantity;side
1716810808663260;1716810808661000;68480.10000000;0.01100000;bid
1716810808667421;1716810808665000;68479.80000000;0.02500000;ask
1716810808671033;1716810808669000;68480.30000000;0.00800000;bid
```

### Выходные данные

Четыре файла:
- `result_price_mean.csv`;
- `result_price_median.csv`;
- `result_price_standart_deviation.csv`;
- `result_price_percentiles.csv`.
В каждом из них просчитана своя метрика. Например `result_price_median.csv`:
```csv
receive_ts;price_median
1716810808595150;68479.750000
1716810808595662;68480.000000
1770656944930394;68480.993750
1770656944930643;103504.224031
```
Также, например `result_price_percentiles.csv`:
```csv
receive_ts;price_percentiles
1716810808595150;68479.750000;68479.750000;68479.750000;68479.750000
1716810808595662;68480.000000;68480.000000;68480.000000;68480.000000
1716810808596701;68480.000000;68480.844444;68480.844444;68480.844444
1716810808597730;68480.000000;68481.825617;68481.825617;68481.825617
1770656944930394;68480.993750;68483.030247;68483.030247;68483.030247
```

### Пример вывода консоли

```bash
[2026-02-13 21:33:32.077] [info] Program starting
[2026-02-13 21:33:32.078] [info] Toml filepath is config.toml
[2026-02-13 21:33:32.078] [info] Parallel processing: false
[2026-02-13 21:33:32.078] [info] Csv input directory: ./examples/input
[2026-02-13 21:33:32.079] [info] Csv output directory: ./examples/output
[2026-02-13 21:33:32.079] [info] Filename masks: ["standart"]
[2026-02-13 21:33:32.079] [info] Located 3 suitable csv files: ["./examples/input\\data_1.csv", "./examples/input\\data_2.csv"]
[2026-02-13 21:33:32.079] [info] Metrics [("price_median", true), ("price_mean", true), ("price_standart_deviation", true), ("price_percentiles", true)]
[2026-02-13 21:33:32.079] [info] Opening csv files...
[2026-02-13 21:33:32.080] [info] Reading file: ./examples/input\data_1.csv
[2026-02-13 21:33:32.081] [info] Reading file: ./examples/input\data_2.csv
[2026-02-13 21:33:32.082] [info] Files parsed: 2
[2026-02-13 21:33:32.082] [info] Data size: 1010 rows
[2026-02-13 21:33:32.083] [info] Calculating metrics...
[2026-02-13 21:33:32.089] [info] Calculated all the metrics and wrote them into result files
[2026-02-13 21:33:32.090] [info] Program finished
```