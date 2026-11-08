#!/usr/bin/env bash
set -Eeuo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "$0")" && pwd)"; source "$SCRIPT_DIR/lib.bash"
nodes= repeat=1 tag=manual windows="$WINDOWS" timing_passes=$TIMING_PASSES; quotes="$DATA_ROOT/input/quotes.csv"
while (( $# )); do case "$1" in
  --nodes) nodes=${2:?}; shift 2;; --repeat) repeat=${2:?}; shift 2;; --tag) tag=${2:?}; shift 2;;
  --windows) windows=${2:?}; shift 2;; --timing-passes) timing_passes=${2:?}; shift 2;; --quotes) quotes=${2:?}; shift 2;;
  *) die "Usage: $0 --nodes N [--repeat R] [--windows '5 10 20'] [--timing-passes N] [--quotes PATH] [--tag NAME]";; esac; done
[[ "$nodes" =~ ^[1-9][0-9]*$ && "$repeat" =~ ^[0-9]+$ && "$timing_passes" =~ ^[1-9][0-9]*$ && -r "$quotes" ]] || die "Invalid arguments or unreadable quotes file"
ready=$(sinfo -h -N -p "$PARTITION" -t idle -o '%N' | wc -l); (( ready >= nodes )) || die "Need $nodes idle workers; found $ready"
mkdirs; run_id="${tag}_n${nodes}_r${repeat}_$(date +%Y%m%dT%H%M%S)_$RANDOM"; run_dir="$RESULTS_ROOT/raw/$run_id"; mkdir -p "$run_dir"
jobid=$(sbatch --parsable --chdir=/tmp --partition="$PARTITION" --nodes="$nodes" --ntasks="$nodes" --ntasks-per-node=1 --cpus-per-task=1 --time="$SLURM_TIME_LIMIT" \
  --export="ALL,LAB_PROJECT_DIR=$PROJECT_DIR,LAB_QUOTES=$quotes,LAB_RUN_DIR=$run_dir,LAB_WINDOWS=$windows,LAB_TEST_FRACTION=$TEST_FRACTION,LAB_TIMING_PASSES=$timing_passes" \
  "$PROJECT_DIR/slurm/forecast.sbatch")
jobid=${jobid%%;*}; echo "Submitted $jobid: $run_id"
while squeue -h -j "$jobid" | grep -q .; do sleep 1; done
state=$(scontrol show job -o "$jobid" 2>/dev/null | sed -n 's/.*JobState=\([^ ]*\).*/\1/p'); state=${state:-UNKNOWN}
elapsed=$(tail -n1 "$run_dir/job_elapsed.csv" 2>/dev/null || true)
metrics="$RESULTS_ROOT/metrics/runs.csv"
[[ -f "$metrics" ]] || echo 'run_id,job_id,status,nodes,repeat,windows,timing_passes,elapsed_seconds,run_dir' > "$metrics"
printf '%s,%s,%s,%s,%s,"%s",%s,%s,%s\n' "$run_id" "$jobid" "$state" "$nodes" "$repeat" "$windows" "$timing_passes" "$elapsed" "$run_dir" >> "$metrics"
[[ "$state" == COMPLETED && -f "$run_dir/forecast_metrics.csv" ]] || die "Run failed or incomplete: $run_id ($state)"
echo "$run_id"
