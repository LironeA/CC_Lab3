param([switch]$ExportResults)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$cluster = 'D:\CSC\6th\S1\CC\claster'
if ($ExportResults) { & "$cluster\cluster.ps1" -Action export; exit $LASTEXITCODE }
if (!(Test-Path "$cluster\exchange\input")) { throw "Cluster exchange directory unavailable: $cluster" }
$archive = Join-Path $env:TEMP 'lab3-market-forecast.tar.gz'
tar -czf $archive -C (Split-Path $root -Parent) (Split-Path $root -Leaf)
Copy-Item $archive "$cluster\exchange\input\lab3-market-forecast.tar.gz" -Force
& "$cluster\cluster.ps1" -Action import
$key = "$cluster\.vagrant\machines\controller\virtualbox\private_key"
ssh -o BatchMode=yes -o StrictHostKeyChecking=accept-new -p 22220 -i $key vagrant@127.0.0.1 "mkdir -p /shared/lab3-market-forecast && tar -xzf /shared/input/lab3-market-forecast.tar.gz -C /shared/lab3-market-forecast --strip-components=1 && chmod +x /shared/lab3-market-forecast/scripts/*.bash /shared/lab3-market-forecast/slurm/*.sbatch"
