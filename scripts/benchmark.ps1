param(
  [int[]]$Threads = @(1..14),
  [int]$Repetitions = 3,
  [int]$TimingPasses = 100000
)
$ErrorActionPreference = 'Stop'
& "$PSScriptRoot\build.ps1"
foreach ($repeat in 1..$Repetitions) {
  foreach ($threadCount in $Threads) {
    & "$PSScriptRoot\run_experiment.ps1" -Threads $threadCount -Repeat $repeat -Tag benchmark -TimingPasses $TimingPasses
    if ($LASTEXITCODE -ne 0) { throw "Benchmark failed at $threadCount threads" }
  }
}
