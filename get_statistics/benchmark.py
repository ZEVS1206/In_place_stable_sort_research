#!/usr/bin/env python3
import random
import subprocess
import time
import csv
import os
import numpy as np
import matplotlib.pyplot as plt

def generate_array_with_density(n, density):
    unique = max(1, int(n * density))
    values = [random.randint(0, unique - 1) for _ in range(n)]
    random.shuffle(values)
    return values

def save_array(arr, fname):
    with open(fname, "w") as f:
        f.write(" ".join(map(str, arr)))

def run_sort(binary, arr, mode):
    fname = "tmp_input.txt"
    save_array(arr, fname)
    t0 = time.perf_counter()
    p = subprocess.run([binary, fname, mode],
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE,
                       text=True)
    dt = time.perf_counter() - t0
    if p.returncode != 0:
        raise RuntimeError(p.stderr)
    return dt

def benchmark(binary,
              sizes,
              densities,
              repeats,
              csv_name="results.csv"):

    with open(csv_name, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["algo", "size", "density", "time"])

        for n in sizes:
            for d in densities:
                for _ in range(repeats):
                    arr = generate_array_with_density(n, d)
                    for algo in ("logsort", "qsort"):
                        try:
                            t = run_sort(binary, arr, algo)
                            w.writerow([algo, n, d, t])
                        except Exception as e:
                            print("ERROR:", e)

        print("✓ benchmark finished:", csv_name)

def plot_3d(csv_name, out_png):
    data = np.genfromtxt(csv_name, delimiter=",", names=True, dtype=None, encoding=None)

    fig = plt.figure(figsize=(11, 7))
    ax = fig.add_subplot(111, projection="3d")

    for algo, color, marker in [
        ("logsort", "blue", "o"),
        ("qsort",   "red",  "^")
    ]:
        sel = data["algo"] == algo
        ax.scatter(
            data["size"][sel],
            data["density"][sel],
            data["time"][sel],
            label=algo,
            c=color,
            marker=marker,
            alpha=0.6
        )

    ax.set_xlabel("Array size")
    ax.set_ylabel("Density (unique / n)")
    ax.set_zlabel("Time, s")
    ax.set_title("logsort vs qsort performance")
    ax.legend()
    plt.tight_layout()
    plt.savefig(out_png, dpi=200, bbox_inches="tight")
    print(f"✓ plot saved to {out_png}")
    plt.show()

if __name__ == "__main__":
    random.seed(123)

    binary = "./test_logsort/build/logsort.exe"

    sizes     = [1_000, 5_000, 10_000, 50_000]
    densities = [0.01, 0.05, 0.1, 0.25, 0.5, 1.0]
    repeats   = 5

    benchmark(binary, sizes, densities, repeats)
    plot_3d("results.csv", out_png="logsort_vs_qsort_3d.png")

