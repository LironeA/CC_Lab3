#!/usr/bin/env python3
"""Download reproducible daily S&P 500 closing values from FRED."""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import urllib.request
from datetime import UTC, datetime
from pathlib import Path


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--metadata", type=Path, required=True)
    parser.add_argument("--symbol", default="SP500")
    parser.add_argument("--start", default="20161107")
    parser.add_argument("--end", default=datetime.now(UTC).strftime("%Y%m%d"))
    args = parser.parse_args()
    # FRED's SP500 series is a daily close series, available from 2016-11-07.
    url = f"https://fred.stlouisfed.org/graph/fredgraph.csv?id={args.symbol}&cos=Close&coed={args.end}"
    with urllib.request.urlopen(url, timeout=60) as response:
        content = response.read()
    rows = list(csv.DictReader(content.decode("utf-8").splitlines()))
    clean = []
    for row in rows:
        try:
            date, close = row["observation_date"], float(row[args.symbol])
            if close > 0 and date.replace("-", "") >= args.start:
                clean.append((date, close))
        except (KeyError, ValueError):
            continue
    if len(clean) < 2500:
        raise SystemExit(f"Only {len(clean)} valid rows downloaded from {url}")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="", encoding="utf-8") as file:
        writer = csv.writer(file); writer.writerow(["Date", "Close"]); writer.writerows(clean)
    digest = hashlib.sha256(args.output.read_bytes()).hexdigest()
    args.metadata.parent.mkdir(parents=True, exist_ok=True)
    args.metadata.write_text(json.dumps({
        "source": "Federal Reserve Economic Data (FRED), SP500 daily close series", "url": url, "symbol": args.symbol,
        "requested_start": args.start, "requested_end": args.end,
        "downloaded_utc": datetime.now(UTC).isoformat(), "rows": len(clean),
        "first_date": clean[0][0], "last_date": clean[-1][0], "sha256": digest,
    }, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Saved {len(clean)} daily quotes to {args.output}")


if __name__ == "__main__":
    main()
