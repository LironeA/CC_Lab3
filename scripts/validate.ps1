param([Parameter(Mandatory = $true)][string]$Run1, [Parameter(Mandatory = $true)][string]$Run2)
$a = Import-Csv (Join-Path $Run1 'forecast_metrics.csv'); $b = Import-Csv (Join-Path $Run2 'forecast_metrics.csv')
if ($a.Count -ne $b.Count) { throw 'Different metric row counts' }
for ($i = 0; $i -lt $a.Count; $i++) {
  if ($a[$i].method -ne $b[$i].method -or $a[$i].window -ne $b[$i].window) { throw 'Different methods or windows' }
  foreach ($field in 'mae', 'rmse', 'mape_percent') { if ([math]::Abs(([double]$a[$i].$field) - ([double]$b[$i].$field)) -gt 1e-8) { throw "Mismatch in $field" } }
}
Write-Host 'Validation passed: results match for different OpenMP thread counts'
