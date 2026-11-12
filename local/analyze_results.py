#!/usr/bin/env python3
"""Aggregate Lab3 timings and create scaling and forecast-accuracy figures."""
from __future__ import annotations
import argparse, csv, shutil, statistics
from collections import defaultdict
from pathlib import Path
import matplotlib.pyplot as plt

def main() -> None:
    parser = argparse.ArgumentParser(); parser.add_argument('results', type=Path); args = parser.parse_args()
    metrics, figures = args.results / 'metrics', args.results / 'figures'; figures.mkdir(exist_ok=True)
    with (metrics / 'runs.csv').open(newline='', encoding='utf-8') as f:
        runs = [r for r in csv.DictReader(f) if r['status'] == 'COMPLETED' and int(r['repeat']) > 0 and r['elapsed_seconds']]
    grouped: dict[int, list[float]] = defaultdict(list)
    for r in runs: grouped[int(r['threads'])].append(float(r['elapsed_seconds']))
    if 1 not in grouped: raise SystemExit('No completed N=1 benchmark run')
    base = statistics.median(grouped[1]); rows = []
    for nodes, values in sorted(grouped.items()):
        elapsed = statistics.median(values); speedup = base / elapsed
        rows.append({'threads': nodes, 'samples': len(values), 'median_seconds': elapsed, 'speedup': speedup, 'efficiency_percent': 100 * speedup / nodes})
    with (metrics / 'aggregate.csv').open('w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0])); writer.writeheader(); writer.writerows(rows)
    for key, title, ylabel, filename in [('median_seconds', 'Execution time', 'Seconds', 'execution_time.png'), ('speedup', 'Speedup', 'S(p)', 'speedup.png'), ('efficiency_percent', 'Efficiency', 'Percent', 'efficiency.png')]:
        plt.figure(figsize=(7,4)); plt.plot([r['threads'] for r in rows], [r[key] for r in rows], marker='o')
        plt.title(title); plt.xlabel('OpenMP threads'); plt.ylabel(ylabel); plt.grid(alpha=.3); plt.tight_layout(); plt.savefig(figures / filename, dpi=160); plt.close()
    # run_dir is an absolute cluster path; exported bundles keep the same run
    # under the local raw/ directory.
    first = args.results / 'raw' / runs[0]['run_id'] / 'forecast_metrics.csv'
    if not first.is_file():
        first = Path(runs[0]['run_dir']) / 'forecast_metrics.csv'
    accuracy = list(csv.DictReader(first.open(newline='', encoding='utf-8')))
    plt.figure(figsize=(7,4))
    for method in sorted({r['method'] for r in accuracy}):
        points = [r for r in accuracy if r['method'] == method]
        plt.plot([int(r['window']) for r in points], [float(r['mae']) for r in points], marker='o', label=method)
    plt.title('Forecast MAE by window'); plt.xlabel('Window, trading days'); plt.ylabel('MAE'); plt.grid(alpha=.3); plt.legend(); plt.tight_layout(); plt.savefig(figures / 'forecast_mae.png', dpi=160); plt.close()
    print(f'Wrote {metrics / "aggregate.csv"} and figures to {figures}')

if __name__ == '__main__': main()
