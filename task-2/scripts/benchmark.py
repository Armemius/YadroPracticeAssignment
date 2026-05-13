import argparse
import matplotlib.pyplot as plt
import random
import statistics
import subprocess
import tempfile
import time
from pathlib import Path

from generator import (
    MAX_MACHINE_COUNT,
    MAX_PRODUCT_TYPE_COUNT,
    require_range,
    write_facility,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Benchmark a simulation executable on generated N=M inputs.")
    parser.add_argument("executable", help="path to the simulation executable")
    parser.add_argument("--min-size", type=int, default=1, help="minimum N=M value")
    parser.add_argument(
        "--max-size", type=int, default=min(MAX_MACHINE_COUNT, MAX_PRODUCT_TYPE_COUNT), help="maximum N=M value"
    )
    parser.add_argument("--step", type=int, default=1, help="step between generated N=M values")
    parser.add_argument("--trials", type=int, default=5, help="number of runs per generated input")
    parser.add_argument("--seed", type=int, default=42, help="base random seed")
    parser.add_argument("-o", "--output", default="benchmark.png", help="output PNG path")
    parser.add_argument("--timeout", type=float, default=30.0, help="timeout per executable run in seconds")
    return parser.parse_args()


def validate_args(args: argparse.Namespace) -> None:
    require_range("min-size", args.min_size, 1, min(MAX_MACHINE_COUNT, MAX_PRODUCT_TYPE_COUNT))
    require_range("max-size", args.max_size, 1, min(MAX_MACHINE_COUNT, MAX_PRODUCT_TYPE_COUNT))
    if args.min_size > args.max_size:
        raise ValueError("min-size must be less than or equal to max-size")
    if args.step <= 0:
        raise ValueError("step must be positive")
    if args.trials <= 0:
        raise ValueError("trials must be positive")


def run_trial(executable: Path, input_file: Path, workdir: Path, timeout: float) -> float:
    started_at = time.perf_counter()
    subprocess.run(
        [str(executable), str(input_file)],
        cwd=workdir,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        check=True,
        timeout=timeout,
    )
    return (time.perf_counter() - started_at) * 1000.0


def save_plot(sizes: list[int], averages_ms: list[float], output: Path) -> None:
    fig, axis = plt.subplots(figsize=(9, 5))
    axis.plot(sizes, averages_ms, marker="o", linewidth=1.8)
    axis.set_title("Facility simulation benchmark")
    axis.set_xlabel("N = M")
    axis.set_ylabel("Average runtime, ms")
    axis.grid(True, linestyle="--", alpha=0.4)
    fig.tight_layout()
    output.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output)
    plt.close(fig)


def benchmark(args: argparse.Namespace) -> tuple[list[int], list[float]]:
    executable = Path(args.executable).resolve()
    if not executable.is_file():
        raise FileNotFoundError(f"executable not found: {executable}")

    sizes = list(range(args.min_size, args.max_size + 1, args.step))
    averages_ms: list[float] = []

    with tempfile.TemporaryDirectory(prefix="facility-bench-") as tmp:
        workdir = Path(tmp)
        for size in sizes:
            input_file = workdir / f"facility-{size}.txt"
            write_facility(input_file, machine_count=size, product_type_count=size, rng=random.Random(args.seed + size))
            measurements = [run_trial(executable, input_file, workdir, args.timeout) for _ in range(args.trials)]
            averages_ms.append(statistics.fmean(measurements))
            print(f"N=M={size}: {statistics.fmean(measurements):.3f} ms")

    return sizes, averages_ms


def main() -> int:
    args = parse_args()
    validate_args(args)
    sizes, averages_ms = benchmark(args)
    save_plot(sizes, averages_ms, Path(args.output))
    print(f"Saved plot to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
