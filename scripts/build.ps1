param()
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$bin = Join-Path $root 'bin'; New-Item -ItemType Directory -Force $bin | Out-Null
g++ -O3 -std=c++17 -fopenmp "$root\src\forecast_omp.cpp" -o "$bin\forecast_omp.exe"
if ($LASTEXITCODE -ne 0) { throw 'OpenMP compilation failed' }
Write-Host "Built $bin\forecast_omp.exe"
