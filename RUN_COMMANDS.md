# Команди виконання

```powershell
cd D:\CSC\6th\S1\CC\Lab3
.\scripts\build.ps1
.\scripts\benchmark.ps1 -Threads (1..14) -Repetitions 3 -TimingPasses 100000
py -m pip install -r local\requirements.txt
py local\analyze_results.py results
```

```powershell
$t1 = (Get-ChildItem results\raw -Directory | Where-Object Name -like 'benchmark_t1_r1_*').FullName
$t14 = (Get-ChildItem results\raw -Directory | Where-Object Name -like 'benchmark_t14_r1_*').FullName
.\scripts\validate.ps1 -Run1 $t1 -Run2 $t14
cd report
latexmk -xelatex -synctex=1 -interaction=nonstopmode -file-line-error main.tex
```
