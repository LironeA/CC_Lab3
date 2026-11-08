#!/usr/bin/env bash
set -Eeuo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd -- "$SCRIPT_DIR/.." && pwd)"
source "${CONFIG_FILE:-$PROJECT_DIR/config/experiment.conf}"
die() { echo "Помилка: $*" >&2; exit 1; }
mkdirs() { mkdir -p "$DATA_ROOT/input" "$RESULTS_ROOT/raw" "$RESULTS_ROOT/metrics"; }
