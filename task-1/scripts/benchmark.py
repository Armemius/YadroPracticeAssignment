import argparse
import math
import statistics
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path
import matplotlib.pyplot as plt
from generator import generate, resources


@dataclass(frozen=True)
class BotRun:
    rooms: int
    score: float


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Benchmarks bot executables on generated dungeons and writes a score chart."
    )
    parser.add_argument("executables", nargs="+", type=Path, help="Bot executables to compare")
    parser.add_argument("-o", "--output", type=Path, default=Path("benchmark.png"), help="Output PNG file")
    parser.add_argument("--min-rooms", type=int, default=5, help="Smallest generated dungeon size")
    parser.add_argument("--max-rooms", type=int, default=250, help="Largest generated dungeon size")
    parser.add_argument("--step", type=int, default=5, help="Dungeon size step")
    parser.add_argument("--trials", type=int, default=3, help="Generated variants per dungeon size")
    parser.add_argument("--seed", type=int, default=42, help="Base random seed")
    parser.add_argument(
        "--food-ratio",
        type=float,
        default=1.0,
        help="Food amount multiplier relative to the number of rooms",
    )
    parser.add_argument(
        "--resource",
        choices=resources,
        default=None,
        help="Target resource for every generated dungeon; random when omitted",
    )
    parser.add_argument("--timeout", type=float, default=300.0, help="Timeout for one bot run in seconds")
    return parser.parse_args()


def validate_args(args: argparse.Namespace) -> None:
    if args.min_rooms < 1:
        raise ValueError("--min-rooms must be positive")
    if args.max_rooms < args.min_rooms:
        raise ValueError("--max-rooms must be greater than or equal to --min-rooms")
    if args.step < 1:
        raise ValueError("--step must be positive")
    if args.trials < 1:
        raise ValueError("--trials must be positive")
    if args.food_ratio <= 0:
        raise ValueError("--food-ratio must be positive")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive")


def executable_label(path: Path) -> str:
    return path.name or str(path)


def parse_result(path: Path) -> int:
    if not path.exists():
        raise RuntimeError(f"{path} was not created")

    result_value: int | None = None
    for line in path.read_text(encoding="utf-8").splitlines():
        parts = line.split()
        if len(parts) == 6 and parts[0] == "result":
            result_value = int(parts[5])

    if result_value is None:
        raise RuntimeError(f"{path} does not contain a result line")
    return result_value


def run_bot(executable: Path, input_text: str, timeout: float) -> int:
    with tempfile.TemporaryDirectory(prefix="tvb-benchmark-") as tmp:
        tmp_path = Path(tmp)
        input_path = tmp_path / "input.txt"
        result_path = tmp_path / "result.txt"
        input_path.write_text(input_text, encoding="utf-8")

        process = subprocess.run(
            [str(executable), str(input_path)],
            cwd=tmp_path,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=timeout,
            check=False,
        )
        if process.returncode != 0:
            message = process.stderr.strip() or process.stdout.strip() or f"exit code {process.returncode}"
            raise RuntimeError(f"{executable} failed: {message}")

        return parse_result(result_path)


def mean_score(scores: list[float]) -> float:
    valid_scores = [score for score in scores if not math.isnan(score)]
    if not valid_scores:
        return float("nan")
    return round(statistics.mean(valid_scores))


def format_score(score: float) -> str:
    if math.isnan(score):
        return "failed"
    return str(round(score))


def room_sizes(min_rooms: int, max_rooms: int, step: int) -> list[int]:
    sizes = list(range(min_rooms, max_rooms + 1, step))
    if sizes[-1] != max_rooms:
        sizes.append(max_rooms)
    return sizes


def benchmark(args: argparse.Namespace) -> dict[str, list[BotRun]]:
    executables = [path.resolve() for path in args.executables]
    for executable in executables:
        if not executable.is_file():
            raise FileNotFoundError(f"Executable not found: {executable}")

    results: dict[str, list[BotRun]] = {executable_label(path): [] for path in executables}
    for rooms in room_sizes(args.min_rooms, args.max_rooms, args.step):
        trial_scores: dict[str, list[int]] = {executable_label(path): [] for path in executables}
        food = max(1, round(rooms * args.food_ratio))

        for trial in range(args.trials):
            seed = args.seed + (rooms * 1009) + trial
            input_text = generate(rooms=rooms, food=food, seed=seed, target_resource=args.resource)

            for executable in executables:
                label = executable_label(executable)
                try:
                    trial_scores[label].append(run_bot(executable, input_text, args.timeout))
                except (RuntimeError, subprocess.TimeoutExpired) as error:
                    trial_scores[label].append(float("nan"))  # type: ignore
                    print(f"{rooms} rooms, trial {trial + 1}, {label}: {error}")

        for label, scores in trial_scores.items():
            results[label].append(BotRun(rooms=rooms, score=mean_score(scores)))  # type: ignore

        summary = ", ".join(f"{label}: {format_score(results[label][-1].score)}" for label in results)
        print(f"{rooms} rooms: {summary}")

    return results


def plot_results(results: dict[str, list[BotRun]], output: Path) -> None:
    plt.figure(figsize=(12, 7))

    for label, runs in results.items():
        x = [run.rooms for run in runs]
        y = [run.score for run in runs]
        plt.plot(x, y, marker="o", linewidth=1.8, markersize=3.5, label=label)

    plt.title("Bot benchmark")
    plt.xlabel("Dungeon size")
    plt.ylabel("Result")
    plt.grid(True, alpha=0.28)
    plt.legend()
    plt.tight_layout()

    output.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(output, dpi=160)
    plt.close()


def main() -> None:
    args = parse_args()
    validate_args(args)
    results = benchmark(args)
    plot_results(results, args.output)
    print(f"Wrote {args.output}")


if __name__ == "__main__":
    main()
