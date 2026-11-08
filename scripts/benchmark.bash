#!/usr/bin/env bash
set -Eeuo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "$0")" && pwd)"; source "$SCRIPT_DIR/lib.bash"
repetitions=3; nodes='1 2 3 4'; timing_passes=$TIMING_PASSES
while (( $# )); do case "$1" in
  --repetitions) repetitions=${2:?}; shift 2;; --nodes) nodes=${2:?}; shift 2;; --timing-passes) timing_passes=${2:?}; shift 2;;
  *) die "Usage: $0 [--repetitions N] [--nodes '1 2 3 4'] [--timing-passes N]";; esac; done
[[ -r "$DATA_ROOT/input/quotes.csv" ]] || die "Run scripts/prepare_data.bash first"
for ((repeat=1; repeat<=repetitions; repeat++)); do
  for count in $nodes; do "$SCRIPT_DIR/run_experiment.bash" --nodes "$count" --repeat "$repeat" --tag benchmark --timing-passes "$timing_passes"; done
done
