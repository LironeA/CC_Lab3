#!/usr/bin/env bash
set -Eeuo pipefail
[[ $# == 2 ]] || { echo "Usage: $0 RUN_DIR_1 RUN_DIR_2" >&2; exit 2; }
python3 - "$1/forecast_metrics.csv" "$2/forecast_metrics.csv" <<'PY'
import csv, sys
def rows(path):
    with open(path, newline='', encoding='utf-8') as f:
        return {(r['method'], r['window']): r for r in csv.DictReader(f)}
a, b = rows(sys.argv[1]), rows(sys.argv[2])
if a.keys() != b.keys(): raise SystemExit('Different method/window sets')
for key in a:
    for field in ('mae', 'rmse', 'mape_percent'):
        if abs(float(a[key][field]) - float(b[key][field])) > 1e-9:
            raise SystemExit(f'Mismatch for {key}: {field}')
print('Validation passed: serial and parallel forecast metrics match')
PY
