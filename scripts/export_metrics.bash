#!/usr/bin/env bash
set -Eeuo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "$0")" && pwd)"; source "$SCRIPT_DIR/lib.bash"
archive="$RESULTS_ROOT/metrics_bundle.tar.gz"
mapfile -t files < <(cd "$RESULTS_ROOT" && find raw -type f \( -name forecast_metrics.csv -o -name summary.json \) -print)
tar -czf "$archive" -C "$RESULTS_ROOT" metrics "${files[@]}"
echo "$archive"
