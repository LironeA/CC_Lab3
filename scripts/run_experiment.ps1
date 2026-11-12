param(
  [Parameter(Mandatory = $true)][int]$Threads,
  [int]$Repeat = 1,
  [string]$Tag = 'manual',
  [int]$TimingPasses = 100000
)
$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $root 'bin\forecast_omp.exe'
$quotes = Join-Path $root 'data\input\quotes.csv'

if (!(Test-Path $exe)) {
    & "$PSScriptRoot\build.ps1"
}

if (!(Test-Path $quotes)) {
    throw 'Quotes are absent. Run src\download_quotes.py first.'
}

$id = '{0}_t{1}_r{2}_{3}' -f $Tag, $Threads, $Repeat, (Get-Date -Format 'yyyyMMddTHHmmssfff')
$run = Join-Path $root "results\raw\$id"

New-Item -ItemType Directory -Force $run | Out-Null
& $exe --quotes $quotes --output-dir $run --threads $Threads --timing-passes $TimingPasses

if ($LASTEXITCODE -ne 0) {
    throw "OpenMP run failed: $id"
}

$summary = Get-Content (Join-Path $run 'summary.json') -Raw | ConvertFrom-Json
$metricsDir = Join-Path $root 'results\metrics'
$csv = Join-Path $metricsDir 'runs.csv'

New-Item -ItemType Directory -Force $metricsDir | Out-Null
if (!(Test-Path $csv)) {
    'run_id,status,threads,repeat,timing_passes,elapsed_seconds,run_dir' | Set-Content $csv
}

"$id,COMPLETED,$Threads,$Repeat,$TimingPasses,$($summary.elapsed_seconds),$run" | Add-Content $csv
Write-Output $id
