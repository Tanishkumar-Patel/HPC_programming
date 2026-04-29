# -*- coding: utf-8 -*-
#!/usr/bin/env python
"""
HPC Assignment 08 - Benchmark Runner
Runs the MPI+OpenMP code for a given input.bin across all core counts (2,4,8,...,64)
with multiple MPI x OMP decompositions, keeps the best (lowest total) time per core
count, and writes results to a CSV.

Usage:
    python run_benchmark.py                  # uses input.bin in current dir
    python run_benchmark.py --input my.bin   # specify input file

    python run_benchmark.py --skip-compile   # skip recompilation
    python run_benchmark.py --label config_a # tag the CSV rows with a label

Typical workflow per config:
    1. ./input_file_maker   (generates input.bin for desired Nx,Ny,points,maxiter)
    2. python run_benchmark.py --label config_a
    3. Repeat for configs b-e with different labels
"""
from __future__ import print_function
import subprocess
import os
import sys
import struct
import argparse
import datetime

# ─── Paths ────────────────────────────────────────────────────────────────────
MPI_PATH = "/usr/mpi/gcc/openmpi-1.8.8/bin"
MPICXX   = os.path.join(MPI_PATH, "mpicxx")
MPIRUN   = os.path.join(MPI_PATH, "mpirun")
HOSTFILE = "sources.txt"        # list of compute nodes (gics1 gics2 gics3 gics4)
EXECUTABLE = "./mpi"            # compiled binary name (matches your existing binary)
CSV_FILE   =  "timing_result.csv"
# ─── Core counts and MPI×OMP decompositions ──────────────────────────────────
# Each entry: total_cores -> [(mpi_ranks, omp_threads), ...]
# mpi_ranks * omp_threads == total_cores for every pair.
CORE_COUNTS = [1, 2, 4, 8, 16, 32, 64]

DECOMPOSITIONS = {
    1:  [(1, 1)],
    2:  [(1, 2), (2, 1)],
    4:  [(1, 4), (2, 2), (4, 1)],
    8:  [(1, 8), (2, 4), (4, 2), (8, 1)],
    16: [(2, 8), (4, 4), (8, 2), (16, 1)],
    32: [(4, 8), (8, 4), (16, 2), (32, 1)],
    64: [(8, 8), (16, 4), (32, 2), (64, 1)],
}

# ─── Helpers ──────────────────────────────────────────────────────────────────

def run_command(cmd, env=None):
    if env is None:
        env = os.environ.copy()
    proc = subprocess.Popen(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env
    )
    stdout, stderr = proc.communicate()
    # decode for Python 3 compatibility
    if isinstance(stdout, bytes):
        stdout = stdout.decode("utf-8", errors="replace")
    if isinstance(stderr, bytes):
        stderr = stderr.decode("utf-8", errors="replace")
    return proc.returncode, stdout, stderr


def compile_program():
    print("=" * 60)
    print("Compiling...")
    cmd = [
        MPICXX, "-fopenmp", "-O3", "-march=native",
        "main.cpp", "utils.cpp", "init.cpp",
        "-o", "mpi"
    ]
    print("  " + " ".join(cmd))
    ret, out, err = run_command(cmd)
    if ret != 0:
        print("Compilation FAILED:\n", err)
        sys.exit(1)
    print("Compilation successful.")
    print("=" * 60)

def parse_output(stdout):
    interp = norm = reverse = mover = None
    for line in stdout.splitlines():
        line = line.strip()
        if "Total Interpolation Time" in line:
            interp = float(line.split("=")[1].replace("seconds","").strip())
        elif "Total Normalization Time" in line:
            norm = float(line.split("=")[1].replace("seconds","").strip())
        elif "Total Mover Time" in line:
            mover = float(line.split("=")[1].replace("seconds","").strip())
        elif "Total Denormalization Time" in line:
            reverse = float(line.split("=")[1].replace("seconds","").strip())
    if None not in (interp, norm, reverse, mover):
        total = interp + norm + reverse + mover
        return total, interp, norm, reverse, mover
    return None, None, None, None, None


def read_grid_info(input_file):
    """Read Nx, Ny, NumPoints, Maxiter from binary input.bin header."""
    with open(input_file, "rb") as f:
        nx       = struct.unpack("<i", f.read(4))[0]
        ny       = struct.unpack("<i", f.read(4))[0]
        npoints  = struct.unpack("<i", f.read(4))[0]
        maxiter  = struct.unpack("<i", f.read(4))[0]
    return nx, ny, npoints, maxiter


def run_config(mpi_ranks, omp_threads, input_file):
    """Run a single MPI+OMP configuration. Returns parsed timing tuple or None."""
    env = os.environ.copy()
    env["OMP_NUM_THREADS"] = str(omp_threads)

    cmd = [MPIRUN, "-np", str(mpi_ranks)]

    # Use hostfile only when running with >1 MPI rank AND the file exists.
    # For single-node runs (ranks <= cores_per_node) you can omit --hostfile.
    if mpi_ranks > 1 and os.path.exists(HOSTFILE):
        cmd += ["--hostfile", HOSTFILE]

    cmd += [EXECUTABLE, input_file, str(omp_threads)]

    print("    CMD: {} | OMP_NUM_THREADS={}".format(" ".join(cmd), omp_threads))
    ret, stdout, stderr = run_command(cmd, env)

    if ret != 0:
        print("    ERROR (exit {}):".format(ret))
        for line in stderr.strip().splitlines()[-5:]:   # last 5 lines of stderr
            print("      " + line)
        return None

    result = parse_output(stdout)
    if result[0] is None:
        print("    [WARN] No timing line found in output.")
        print("    stdout:", stdout[:300])
        return None

    print("    -> Total={:.4f}s  Interp={:.4f}s  Norm={:.4f}s  "
          "Reverse={:.4f}s  Mover={:.4f}s".format(*result))
    return result


# ─── Main benchmark loop ──────────────────────────────────────────────────────

def benchmark(input_file, label, skip_compile):
    global CSV_FILE
    CSV_FILE = label + ".csv"
    if not os.path.exists(input_file):
        print("Input file '{}' not found. Generate it first with input_file_maker.".format(input_file))
        sys.exit(1)

    if not skip_compile:
        compile_program()
    else:
        print("Skipping compilation (--skip-compile).")

    if not os.path.exists(EXECUTABLE):
        print("Executable '{}' not found. Compile first.".format(EXECUTABLE))
        sys.exit(1)

    nx, ny, npoints, maxiter = read_grid_info(input_file)
    print("\nGrid info from {}: Nx={}, Ny={}, Points={}, Maxiter={}".format(
        input_file, nx, ny, npoints, maxiter))

    # Initialise CSV (write header if file doesn't exist)
    write_header = not os.path.exists(CSV_FILE)
    with open(CSV_FILE, "a") as f:
        if write_header:
            f.write("label,timestamp,nx,ny,npoints,maxiter,"
                    "total_cores,best_mpi,best_omp,"
                    "total_time,interp,norm,reverse,mover\n")

    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    # ── Serial baseline (1 core) ──
    print("\n" + "=" * 60)
    print("=== Serial baseline (1 MPI rank, 1 OMP thread) ===")
    res = run_config(1, 1, input_file)
    if res is None:
        print("Serial run failed — aborting.")
        sys.exit(1)
    serial_time = res[0]
    with open(CSV_FILE, "a") as f:
        f.write("{},{},{},{},{},{},1,1,1,{:.6f},{:.6f},{:.6f},{:.6f},{:.6f}\n".format(
            label, timestamp, nx, ny, npoints, maxiter, *res))
    print("  Serial total time: {:.4f}s".format(serial_time))

    # ── Parallel runs ──
    for cores in CORE_COUNTS[1:]:   # skip 1 (already done above)
        print("\n" + "=" * 60)
        print("=== Total cores = {} ===".format(cores))

        best_time = None
        best_data = None

        for mpi, omp in DECOMPOSITIONS.get(cores, []):
            print("  Trying MPI={}, OMP={}:".format(mpi, omp))
            res = run_config(mpi, omp, input_file)
            if res is not None:
                t = res[0]
                if best_time is None or t < best_time:
                    best_time = t
                    best_data = (mpi, omp) + res   # (mpi, omp, total, interp, norm, rev, mover)

        if best_data:
            mpi, omp = best_data[0], best_data[1]
            speedup = serial_time / best_data[2] if best_data[2] > 0 else 0
            efficiency = speedup / cores * 100
            print("  >> BEST for {} cores: MPI={}, OMP={}, time={:.4f}s, "
                  "speedup={:.2f}x, efficiency={:.1f}%".format(
                      cores, mpi, omp, best_data[2], speedup, efficiency))
            with open(CSV_FILE, "a") as f:
                f.write("{},{},{},{},{},{},{},{},{},{:.6f},{:.6f},{:.6f},{:.6f},{:.6f}\n".format(
                    label, timestamp, nx, ny, npoints, maxiter,
                    cores, mpi, omp, *best_data[2:]))
        else:
            print("  >> No successful run for {} cores.".format(cores))

    print("\n" + "=" * 60)
    print("Benchmark complete. Results appended to '{}'.".format(CSV_FILE))
    print("=" * 60)


# ─── Entry point ──────────────────────────────────────────────────────────────

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Benchmark MPI+OMP HPC code across all core decompositions."
    )
    parser.add_argument(
        "--input", default="input.bin",
        help="Path to input binary file (default: input.bin)"
    )
    parser.add_argument(
        "--label", default="config",
        help="Label for this config in the CSV, e.g. config_a (default: config)"
    )
    parser.add_argument(
        "--skip-compile", action="store_true",
        help="Skip recompilation (use existing binary)"
    )
    args = parser.parse_args()

    benchmark(
        input_file=args.input,
        label=args.label,
        skip_compile=args.skip_compile,
    )
