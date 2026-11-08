# Команди виконання

```powershell
cd D:\CSC\6th\S1\CC\claster
.\cluster.ps1 -Action start
cd D:\CSC\6th\S1\CC\Lab3
.\windows\deploy.ps1
```

```bash
cd /shared/lab3-market-forecast
python3 -m pip install --user -r requirements.txt
scripts/prepare_data.bash
scripts/run_experiment.bash --nodes 1 --repeat 0 --tag validation
scripts/run_experiment.bash --nodes 2 --repeat 0 --tag validation
scripts/validate.bash results/raw/VALIDATION_N1 results/raw/VALIDATION_N2
scripts/benchmark.bash --repetitions 3
scripts/export_metrics.bash
```

```powershell
.\windows\deploy.ps1 -ExportResults
py -m pip install -r local\requirements.txt
py local\analyze_results.py D:\CSC\6th\S1\CC\claster\exchange\output
cd report
latexmk -xelatex -synctex=1 -interaction=nonstopmode -file-line-error main.tex
```
