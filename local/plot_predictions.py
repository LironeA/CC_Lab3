#!/usr/bin/env python3
"""Plot actual test prices and one-step forecasts for the best MAE model."""
from __future__ import annotations

import argparse
import csv
from pathlib import Path

import matplotlib.dates as mdates
import matplotlib.pyplot as plt
import numpy as np


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--quotes", type=Path, required=True)
    parser.add_argument("--metrics", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--test-fraction", type=float, default=.2)
    args = parser.parse_args()
    with args.quotes.open(newline="", encoding="utf-8") as file:
        quotes = [
            (row["Date"], float(row["Close"]))
            for row in csv.DictReader(file)
            if row["Close"]
        ]
    with args.metrics.open(newline="", encoding="utf-8") as file:
        metrics = list(csv.DictReader(file))
    best = min(metrics, key=lambda row: float(row["mae"]))
    method = best["method"]
    window = int(best["window"])
    dates = np.asarray([np.datetime64(date) for date, _ in quotes])
    prices = np.asarray([price for _, price in quotes], dtype=float)
    start = max(window, int(len(prices) * (1 - args.test_fraction)))
    indexes = np.arange(start, len(prices))
    if method == "SMA":
        prediction = np.array([prices[i-window:i].mean() for i in indexes])
    else:
        weights = np.arange(1, window + 1)
        prediction = np.array([np.dot(prices[i-window:i], weights) / weights.sum() for i in indexes])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    plt.figure(figsize=(10, 4.8))
    plt.plot(
        dates[indexes],
        prices[indexes],
        label="Фактична ціна закриття",
        color="#1f77b4",
        linewidth=1.5,
    )
    plt.plot(
        dates[indexes],
        prediction,
        label=f"Прогноз {method}({window})",
        color="#d62728",
        linewidth=1.25,
        alpha=.9,
    )
    plt.title(f"Однокроковий прогноз: найкраща модель {method}({window}), MAE={float(best['mae']):.3f}")
    plt.xlabel("Дата")
    plt.ylabel("Значення S&P 500")
    plt.gca().xaxis.set_major_locator(mdates.MonthLocator(interval=3))
    plt.gca().xaxis.set_major_formatter(mdates.DateFormatter("%Y-%m"))
    plt.xticks(rotation=35, ha="right")
    plt.grid(alpha=.25)
    plt.legend()
    plt.tight_layout()
    plt.savefig(args.output, dpi=180)
    plt.close()
    print(f"Wrote {args.output}; best model: {method}({window})")


if __name__ == "__main__":
    main()
