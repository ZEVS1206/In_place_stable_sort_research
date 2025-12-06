#!/usr/bin/env python3
import random
import subprocess
import time
import csv
import numpy as np
import matplotlib.pyplot as plt
from collections import Counter

def generate_array_with_density(n, target_density):
    """Генерирует массив с примерной целевой плотностью уникальных элементов"""
    target_unique = max(1, int(n * target_density))
    
    values = [random.randint(0, target_unique - 1) for _ in range(n)]
    random.shuffle(values)
    return values

def calculate_actual_density(arr):
    """Вычисляет реальную плотность уникальных элементов"""
    unique_count = len(set(arr))
    n = len(arr)
    return unique_count / n if n > 0 else 0

def save_array(arr, fname):
    with open(fname, "w") as f:
        f.write(" ".join(map(str, arr)))

def run_sort(binary, arr, mode):
    fname = "statistics/tmp_input.txt"
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
              target_densities,
              repeats,
              csv_name="statistics/results.csv"):
    
    with open(csv_name, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["algo", "size", "target_density", "actual_density", "time", "actual_unique", "total"])
        
        for n in sizes:
            for target_d in target_densities:
                for _ in range(repeats):
                    arr = generate_array_with_density(n, target_d)
                    actual_d = calculate_actual_density(arr)
                    actual_unique = len(set(arr))
                    
                    for algo in ("logsort", "qsort"):
                        try:
                            t = run_sort(binary, arr, algo)
                            w.writerow([algo, n, target_d, actual_d, t, actual_unique, n])
                            f.flush()
                        except Exception as e:
                            print(f"ERROR for {algo}, n={n}, target_d={target_d}: {e}")
                            w.writerow([algo, n, target_d, actual_d, -1, actual_unique, n])
        
        print(f"✓ Benchmark finished: {csv_name}")

def analyze_density_discrepancy(csv_name):
    """Анализирует расхождение между целевой и реальной плотностью"""
    data = np.genfromtxt(csv_name, delimiter=",", names=True, dtype=None, encoding=None)
    
    print("\n=== Анализ плотности ===")
    print(f"Всего записей: {len(data)}")
    
    target_density = data['target_density']
    actual_density = data['actual_density']
    
    diff = np.abs(actual_density - target_density)
    print(f"Среднее отклонение: {np.mean(diff):.4f}")
    print(f"Максимальное отклонение: {np.max(diff):.4f}")
    print(f"Минимальное отклонение: {np.min(diff):.4f}")
    
    unique_targets = np.unique(target_density)
    for td in unique_targets:
        mask = target_density == td
        avg_actual = np.mean(actual_density[mask])
        print(f"Целевая {td:.3f} -> Средняя реальная {avg_actual:.3f}")

def plot_3d_by_target(csv_name, out_png_prefix="statistics/logsort_vs_qsort"):
    """Строит графики по целевой плотности"""
    data = np.genfromtxt(csv_name, delimiter=",", names=True, dtype=None, encoding=None)
    
    # График 1: по целевой плотности
    fig = plt.figure(figsize=(11, 7))
    ax = fig.add_subplot(111, projection="3d")
    
    for algo, color, marker in [
        ("logsort", "blue", "o"),
        ("qsort",   "red",  "^")
    ]:
        sel = data["algo"] == algo
        ax.scatter(
            data["size"][sel],
            data["target_density"][sel],
            data["time"][sel],
            label=algo,
            c=color,
            marker=marker,
            alpha=0.6
        )
    
    ax.set_xlabel("Размер массива")
    ax.set_ylabel("Целевая плотность")
    ax.set_zlabel("Время, с")
    ax.set_title("Производительность logsort vs qsort (по целевой плотности)")
    ax.legend()
    plt.tight_layout()
    plt.savefig(f"{out_png_prefix}_target.png", dpi=200, bbox_inches="tight")
    print(f"✓ Target density graph: {out_png_prefix}_target.png")
    plt.show()
    
    # График 2: по реальной плотности
    fig = plt.figure(figsize=(11, 7))
    ax = fig.add_subplot(111, projection="3d")
    
    for algo, color, marker in [
        ("logsort", "blue", "o"),
        ("qsort",   "red",  "^")
    ]:
        sel = data["algo"] == algo
        ax.scatter(
            data["size"][sel],
            data["actual_density"][sel],
            data["time"][sel],
            label=algo,
            c=color,
            marker=marker,
            alpha=0.6
        )
    
    ax.set_xlabel("Размер массива")
    ax.set_ylabel("Реальная плотность")
    ax.set_zlabel("Время, с")
    ax.set_title("Производительность logsort vs qsort (по реальной плотности)")
    ax.legend()
    plt.tight_layout()
    plt.savefig(f"{out_png_prefix}_actual.png", dpi=200, bbox_inches="tight")
    print(f"✓ Real density graph: {out_png_prefix}_actual.png")
    plt.show()
    
    # График 3: Отношение времени logsort / qsort
    fig = plt.figure(figsize=(14, 6))
    
    results = {}
    for row in data:
        key = (row['size'], row['target_density'], row['actual_density'])
        if key not in results:
            results[key] = {}
        results[key][row['algo']] = row['time']
    
    ratios = []
    sizes = []
    densities = []
    
    for key, times in results.items():
        if 'logsort' in times and 'qsort' in times and times['qsort'] > 0:
            size, target_d, actual_d = key
            ratio = times['logsort'] / times['qsort']
            ratios.append(ratio)
            sizes.append(size)
            densities.append(actual_d)
    
    ax = fig.add_subplot(121)
    scatter = ax.scatter(sizes, densities, c=ratios, cmap='RdYlGn', 
                        vmin=0.5, vmax=2.0, s=50, alpha=0.7)
    ax.set_xlabel("Размер массива")
    ax.set_ylabel("Реальная плотность")
    ax.set_title("Отношение времени logsort / qsort")
    plt.colorbar(scatter, label='logsort / qsort')
    ax.grid(True, alpha=0.3)
    
    ax2 = fig.add_subplot(122)
    ax2.hist(ratios, bins=20, alpha=0.7, color='steelblue', edgecolor='black')
    ax2.axvline(x=1.0, color='red', linestyle='--', label='logsort = qsort')
    ax2.set_xlabel("Отношение logsort / qsort")
    ax2.set_ylabel("Частота")
    ax2.set_title(f"Распределение отношений (среднее: {np.mean(ratios):.3f})")
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(f"{out_png_prefix}_ratio.png", dpi=200, bbox_inches="tight")
    print(f"✓ Timeline of the lead time relationship: {out_png_prefix}_ratio.png")
    plt.show()

def generate_array_with_exact_density(n, target_density):
    """Генерирует массив с ТОЧНОЙ целевой плотностью уникальных элементов"""
    exact_unique = max(1, round(n * target_density))
    
    unique_values = list(range(exact_unique))
    
    if n > exact_unique:
        values = unique_values.copy()
        while len(values) < n:
            values.append(random.choice(unique_values))
    else:
        values = unique_values[:n]
    
    random.shuffle(values)
    return values

def run_detailed_analysis(binary, sizes, densities, repeats=3):
    """Запускает детальный анализ производительности"""
    all_results = []
    
    for n in sizes:
        for d in densities:
            print(f"\nТестируем: n={n}, density={d}")
            
            for rep in range(repeats):
                arr_approx = generate_array_with_density(n, d)
                actual_d = calculate_actual_density(arr_approx)
                
                arr_exact = generate_array_with_exact_density(n, d)
                exact_actual_d = calculate_actual_density(arr_exact)
                
                for algo in ("logsort", "qsort"):
                    try:
                        t_approx = run_sort(binary, arr_approx, algo)
                        t_exact = run_sort(binary, arr_exact, algo)
                        
                        all_results.append({
                            'algo': algo,
                            'size': n,
                            'target_density': d,
                            'approx_density': actual_d,
                            'exact_density': exact_actual_d,
                            'time_approx': t_approx,
                            'time_exact': t_exact
                        })
                        
                        print(f"  {algo}: approx={t_approx:.6f}s (d={actual_d:.3f}), "
                              f"exact={t_exact:.6f}s (d={exact_actual_d:.3f})")
                              
                    except Exception as e:
                        print(f"  ERROR for {algo}: {e}")
    
    return all_results

if __name__ == "__main__":
    random.seed(123)
    
    binary = "./test_logsort/build/logsort.exe"
    
    sizes     = [1000, 5000, 10000, 20000, 100000, 200000, 300000, 600000, 700000, 800000, 1000000]
    densities = [0.01, 0.05, 0.1, 0.25, 0.5, 0.75, 1.0]
    repeats = 3
    
    print("=== Start general benchmark ===")
    benchmark(binary, sizes, densities, repeats, "statistics/results_detailed.csv")
    
    print("\n=== Analyze density ===")
    analyze_density_discrepancy("statistics/results_detailed.csv")
    
    print("\n=== Create graphs ===")
    plot_3d_by_target("statistics/results_detailed.csv")
    
    # # Опционально: детальный анализ
    # if input("\nЗапустить детальный анализ? (y/n): ").lower() == 'y':
    #     print("\n=== Запуск детального анализа ===")
    #     detailed_results = run_detailed_analysis(binary, sizes[:3], densities[:3], repeats=2)
        
    #     # Анализ разницы между примерной и точной генерацией
    #     if detailed_results:
    #         print("\n=== Сравнение примерной и точной генерации ===")
    #         for algo in ("logsort", "qsort"):
    #             algo_results = [r for r in detailed_results if r['algo'] == algo]
    #             if algo_results:
    #                 avg_ratio = np.mean([r['time_exact'] / r['time_approx'] 
    #                                     for r in algo_results if r['time_approx'] > 0])
    #                 print(f"{algo}: среднее отношение exact/approx = {avg_ratio:.3f}")