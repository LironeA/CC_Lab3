# Лабораторна №3 — паралельний прогноз біржових котирувань

Проєкт виконує однокроковий прогноз денних цін закриття індексу S\&P 500 за простим рухомим середнім (SMA) та лінійно зваженим рухомим середнім (WMA). Розрахунок похибок і прогнозів розподілено між MPI-процесами; запуск виконується через Slurm-кластер з Lab1--Lab2.

## Дані та методика

`scripts/prepare_data.bash` завантажує денний ряд `SP500` із Federal Reserve Economic Data (FRED) за 2016--2026 роки. Це допустимий десятирічний інтервал із умови. У `data/input/quotes_metadata.json` зберігаються URL, часові межі, кількість точок та SHA-256 CSV. Для прогнозу в момент $t$ використовуються лише ціни $t-N,...,t-1$; останні 20\% ряду є тестовою вибіркою. Оцінюються MAE, RMSE і MAPE. Вікна: 5, 10, 20, 30, 60, 120, 252 торгових дні.

У WMA найновіша ціна має вагу $N$, а найдавніша --- 1. Найкраще вікно для кожного методу визначається мінімумом MAE на тестовій частині. Прогноз наступного кроку та найкращі параметри записуються до `summary.json`.

## Розгортання та запуск

```powershell
cd D:\CSC\6th\S1\CC\Lab3
.\windows\deploy.ps1
```

На controller:

```bash
cd /shared/lab3-market-forecast
python3 -m pip install --user -r requirements.txt
scripts/prepare_data.bash
scripts/run_experiment.bash --nodes 1 --repeat 0 --tag validation
scripts/run_experiment.bash --nodes 2 --repeat 0 --tag validation
# Передайте два створені каталоги до validate.bash.
scripts/benchmark.bash --repetitions 3 --nodes '1 2 3 4'
scripts/export_metrics.bash
```

`TIMING_PASSES=10000` у `config/experiment.conf` повторює однакове обчислювальне ядро в межах одного виміру. Зведення MPI виконується один раз після серії проходів, тому вимірюється прогнозне обчислення, а не затримка мережі. Це необхідно, бо один прохід над приблизно 2.5 тис. денних значень замалий для достовірного вимірювання MPI-масштабування. Завантаження CSV, очікування у черзі Slurm і запис результатів не входять до вимірюваного інтервалу.

Після `windows/deploy.ps1 -ExportResults` локально:

```powershell
py -m pip install -r local\requirements.txt
py local\analyze_results.py D:\CSC\6th\S1\CC\claster\exchange\output
```

Отримуються `metrics/aggregate.csv` і графіки `execution_time.png`, `speedup.png`, `efficiency.png`, `forecast_mae.png`.

## Перевірка

`scripts/validate.bash RUN_N1 RUN_N2` порівнює MAE, RMSE і MAPE між послідовним та паралельним запуском з точністю `1e-9`. Перевірка Bash: `bash -n scripts/*.bash slurm/*.sbatch`.
