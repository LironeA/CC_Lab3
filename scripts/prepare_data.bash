#!/usr/bin/env bash
set -Eeuo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "$0")" && pwd)"; source "$SCRIPT_DIR/lib.bash"
symbol=$SYMBOL; start=$START_DATE; end=$END_DATE
while (( $# )); do case "$1" in
  --symbol) symbol=${2:?}; shift 2;; --start) start=${2:?}; shift 2;; --end) end=${2:?}; shift 2;;
  *) die "Usage: $0 [--symbol SP500] [--start YYYYMMDD] [--end YYYYMMDD]";; esac; done
mkdirs
python3 "$PROJECT_DIR/src/download_quotes.py" --output "$DATA_ROOT/input/quotes.csv" --metadata "$DATA_ROOT/input/quotes_metadata.json" --symbol "$symbol" --start "$start" --end "$end"
