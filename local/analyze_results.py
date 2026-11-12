#!/usr/bin/env python3
"""Aggregate Lab3 timings and create scaling and forecast-accuracy figures."""
from __future__ import annotations

import argparse
import csv
import statistics
from collections import defaultdict
from pathlib import Path
import matplotlib.pyplot as plt


def load_completed_runs(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as file:
        return [
            row
            for row in csv.DictReader(file)
            if row["status"] == "COMPLETED" and int(row["repeat"]) > 0 and row["elapsed_seconds"]
        ]


def write_scaling_figures(rows: list[dict[str, float]], figures: Path) -> None:
    plots = [
        ("median_seconds", "Execution time", "Seconds", "execution_time.png"),
        ("speedup", "Speedup", "S(p)", "speedup.png"),
        ("efficiency_percent", "Efficiency", "Percent", "efficiency.png"),
    ]
    for key, title, ylabel, filename in plots:
        plt.figure(figsize=(7, 4))
        plt.plot([row["threads"] for row in rows], [row[key] for row in rows], marker="o")
        plt.title(title)
        plt.xlabel("OpenMP threads")
        plt.ylabel(ylabel)
        plt.grid(alpha=0.3)
        plt.tight_layout()
        plt.savefig(figures / filename, dpi=160)
        plt.close()


def write_accuracy_figure(metrics_file: Path, figures: Path) -> None:
    with metrics_file.open(newline="", encoding="utf-8") as file:
        accuracy = list(csv.DictReader(file))

    plt.figure(figsize=(7, 4))
    for method in sorted({row["method"] for row in accuracy}):
        points = [row for row in accuracy if row["method"] == method]
        plt.plot(
            [int(row["window"]) for row in points],
            [float(row["mae"]) for row in points],
            marker="o",
            label=method,
        )
    plt.title("Forecast MAE by window")
    plt.xlabel("Window, trading days")
    plt.ylabel("MAE")
    plt.grid(alpha=0.3)
    plt.legend()
    plt.tight_layout()
    plt.savefig(figures / "forecast_mae.png", dpi=160)
    plt.close()

def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("results", type=Path)
    args = parser.parse_args()

    metrics = args.results / "metrics"
    figures = args.results / "figures"
    figures.mkdir(exist_ok=True)
    runs = load_completed_runs(metrics / "runs.csv")
    grouped: dict[int, list[float]] = defaultdict(list)
    for run in runs:
        grouped[int(run["threads"])].append(float(run["elapsed_seconds"]))
    if 1 not in grouped:
        raise SystemExit("No completed single-thread benchmark run")

    serial_time = statistics.median(grouped[1])
    rows: list[dict[str, float]] = []
    for thread_count, elapsed_values in sorted(grouped.items()):
        median_time = statistics.median(elapsed_values)
        speedup = serial_time / median_time
        rows.append({
            "threads": thread_count,
            "samples": len(elapsed_values),
            "median_seconds": median_time,
            "speedup": speedup,
            "efficiency_percent": 100 * speedup / thread_count,
        })

    with (metrics / "aggregate.csv").open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)

    write_scaling_figures(rows, figures)
    first_metrics = args.results / "raw" / runs[0]["run_id"] / "forecast_metrics.csv"
    write_accuracy_figure(first_metrics, figures)
    print(f'Wrote {metrics / "aggregate.csv"} and figures to {figures}')

if __name__ == "__main__":
    main()
