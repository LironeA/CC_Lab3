#!/usr/bin/env bash
#SBATCH --job-name=lab3-forecast
#SBATCH --partition=compute
set -Eeuo pipefail
: "${LAB_PROJECT_DIR:?}" "${LAB_QUOTES:?}" "${LAB_RUN_DIR:?}" "${LAB_WINDOWS:?}"
mkdir -p "$LAB_RUN_DIR"
start=$(date +%s%N)
# OpenMPI in the educational cluster must use the host network, not Vagrant NAT.
mpirun --bind-to none --mca oob_tcp_if_include eth1 --mca btl_tcp_if_include eth1 -np "$SLURM_NTASKS" \
  python3 "$LAB_PROJECT_DIR/src/forecast_mpi.py" --quotes "$LAB_QUOTES" --output-dir "$LAB_RUN_DIR" \
  --windows $LAB_WINDOWS --test-fraction "$LAB_TEST_FRACTION" --timing-passes "$LAB_TIMING_PASSES"
end=$(date +%s%N)
awk -v a="$start" -v b="$end" 'BEGIN { printf "elapsed_seconds\n%.9f\n", (b-a)/1000000000 }' > "$LAB_RUN_DIR/job_elapsed.csv"
