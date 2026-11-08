#!/usr/bin/env python3
"""MPI calculation of one-step SMA and linearly weighted MA forecasts."""
from __future__ import annotations

import argparse
import csv
import json
import time
from pathlib import Path

import numpy as np
from mpi4py import MPI


def load_quotes(path: Path) -> tuple[list[str], np.ndarray]:
    dates: list[str] = []; values: list[float] = []
    with path.open(newline="", encoding="utf-8") as file:
        for row in csv.DictReader(file):
            try:
                value = float(row["Close"])
                if value > 0:
                    dates.append(row["Date"]); values.append(value)
            except (KeyError, ValueError):
                pass
    if len(values) < 20:
        raise ValueError("The quote file has too few valid closing prices")
    return dates, np.asarray(values, dtype=np.float64)


def local_statistics(values: np.ndarray, window: int, start: int, stop: int, rank: int, size: int) -> np.ndarray:
    """Return count, absolute-error sum, squared-error sum, percentage-error sum."""
    indexes = np.arange(start + rank, stop, size, dtype=np.int64)
    if indexes.size == 0:
        return np.zeros((2, 4), dtype=np.float64)
    prefix = np.concatenate(([0.0], np.cumsum(values)))
    weighted = np.concatenate(([0.0], np.cumsum(values * np.arange(1, len(values) + 1))))
    left = indexes - window
    sums = prefix[indexes] - prefix[left]
    sma = sums / window
    weighted_sums = (weighted[indexes] - weighted[left]) - left * sums
    wma = weighted_sums / (window * (window + 1) / 2)
    actual = values[indexes]
    result = np.empty((2, 4), dtype=np.float64)
    for method, forecast in enumerate((sma, wma)):
        errors = forecast - actual
        result[method] = (len(actual), np.abs(errors).sum(), np.square(errors).sum(), (np.abs(errors) / actual).sum())
    return result


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--quotes", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--windows", nargs="+", type=int, default=[5, 10, 20, 30, 60, 120, 252])
    parser.add_argument("--test-fraction", type=float, default=.2)
    parser.add_argument("--timing-passes", type=int, default=1)
    args = parser.parse_args()
    if not 0 < args.test_fraction < 1 or args.timing_passes < 1 or any(w < 1 for w in args.windows):
        raise SystemExit("Invalid test fraction, timing passes, or windows")
    comm = MPI.COMM_WORLD; rank, size = comm.Get_rank(), comm.Get_size()
    if rank == 0:
        dates, values = load_quotes(args.quotes)
        payload = (dates, values)
    else:
        payload = None
    dates, values = comm.bcast(payload, root=0)
    test_start = max(max(args.windows), int(len(values) * (1 - args.test_fraction)))
    if test_start >= len(values):
        raise SystemExit("Not enough quotes for the selected windows")
    comm.Barrier(); started = time.perf_counter()
    output: list[dict[str, object]] = []
    for window in sorted(set(args.windows)):
        reduced = np.zeros((2, 4), dtype=np.float64)
        # Repeated passes make the very short kernel measurable.  Reduction is
        # deliberately done once: it is not part of the arithmetic kernel and
        # repeating it would measure network latency rather than forecasting.
        for _ in range(args.timing_passes):
            local = local_statistics(values, window, test_start, len(values), rank, size)
        comm.Reduce(local, reduced, op=MPI.SUM, root=0)
        if rank == 0:
            for method, name in enumerate(("SMA", "WMA")):
                count, absolute, squared, percentage = reduced[method]
                output.append({"method": name, "window": window, "samples": int(count),
                               "mae": absolute / count, "rmse": (squared / count) ** .5,
                               "mape_percent": 100 * percentage / count})
    comm.Barrier(); elapsed = time.perf_counter() - started
    if rank == 0:
        args.output_dir.mkdir(parents=True, exist_ok=True)
        with (args.output_dir / "forecast_metrics.csv").open("w", newline="", encoding="utf-8") as file:
            writer = csv.DictWriter(file, fieldnames=list(output[0])); writer.writeheader(); writer.writerows(output)
        best = {method: min((r for r in output if r["method"] == method), key=lambda r: float(r["mae"])) for method in ("SMA", "WMA")}
        last_predictions = {}
        for method, row in best.items():
            w = int(row["window"]); segment = values[-w:]
            last_predictions[method] = float(segment.mean() if method == "SMA" else np.dot(segment, np.arange(1, w + 1)) / (w * (w + 1) / 2))
        summary = {"symbol_observations": len(values), "first_date": dates[0], "last_date": dates[-1],
                   "test_start_date": dates[test_start], "test_samples": len(values) - test_start,
                   "mpi_processes": size, "timing_passes": args.timing_passes, "elapsed_seconds": elapsed,
                   "best_by_mae": best, "next_step_predictions": last_predictions}
        (args.output_dir / "summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        print(json.dumps(summary, ensure_ascii=False))


if __name__ == "__main__":
    main()
