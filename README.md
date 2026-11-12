# Лабораторна №3 — OpenMP-прогноз біржових котирувань

Локальний C++17/OpenMP-проєкт для однокрокового прогнозування денного індексу S\&P 500 за простим (SMA) і лінійно зваженим (WMA) рухомими середніми. Розрахунки виконуються спільною пам'яттю на ядрах одного ПК; MPI, Slurm і віртуальні машини не використовуються.

## Дані та модель

`data/input/quotes.csv` містить 2513 денних значень індексу S\&P 500 за 2016--2026 роки, одержаних із FRED. Метадані, URL і SHA-256 наведено в `data/input/quotes_metadata.json`. Останні 20\% ряду є тестовою частиною. Оцінюються MAE, RMSE та MAPE для вікон 5, 10, 20, 30, 60, 120 і 252 торгових дні.

Усі OpenMP-потоки працюють з одним спільним масивом цін. Директива `#pragma omp parallel for` розподіляє незалежні прогнози для різних тестових дат між потоками, а `reduction` об'єднує локальні суми похибок.

## Збірка та запуск

Потрібен MinGW-w64 `g++` із підтримкою OpenMP. У поточному середовищі використано GCC 13.2.0.

```powershell
cd D:\CSC\6th\S1\CC\Lab3
.\scripts\build.ps1
.\bin\forecast_omp.exe --quotes data\input\quotes.csv --output-dir results\manual --threads 4 --timing-passes 100000
```

Для повного benchmark на 14 фізичних ядрах Intel Core i5-14600K:

```powershell
.\scripts\benchmark.ps1 -Threads (1..14) -Repetitions 3 -TimingPasses 100000
py -m pip install -r local\requirements.txt
py local\analyze_results.py results
```

`results/metrics/aggregate.csv` містить медіани часу, прискорення й ефективність. Графіки створюються в `results/figures/`.

## Перевірка

Метрики для різної кількості потоків мають збігатися:

```powershell
.\scripts\validate.ps1 -Run1 results\raw\RUN_T1 -Run2 results\raw\RUN_T14
```

## Звіт

Звіт і зібраний PDF: `report/main.tex`, `report/main.pdf`. Він використовує шаблон із `D:\CSC\Template`.
