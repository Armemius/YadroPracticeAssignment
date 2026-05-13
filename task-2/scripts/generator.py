import argparse
import random
from pathlib import Path

MAX_MACHINE_COUNT = 100
MAX_PRODUCT_TYPE_COUNT = 100
MAX_OPERATION_TIME = 10_000
MAX_TOTAL_PRODUCTS = 100_000


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate a facility simulation input file.")
    parser.add_argument("N", type=int, help="number of machines")
    parser.add_argument("M", type=int, help="number of product types")
    parser.add_argument("seed", type=int, nargs="?", default=42, help="random seed")
    parser.add_argument("output", nargs="?", default="facility.txt", help="output file path")
    return parser.parse_args()


def require_range(name: str, value: int, min_value: int, max_value: int) -> None:
    if value < min_value or value > max_value:
        raise ValueError(f"{name} must be in [{min_value}, {max_value}], got {value}")


def generate_operation_times(rng: random.Random, machine_count: int, product_type_count: int) -> list[list[int]]:
    return [[rng.randint(0, MAX_OPERATION_TIME) for _ in range(machine_count)] for _ in range(product_type_count - 1)]


def generate_queues(rng: random.Random, machine_count: int, product_type_count: int) -> list[list[int]]:
    if product_type_count == 1:
        return [[] for _ in range(machine_count)]

    queues: list[list[int]] = []
    total_products = 0
    max_queue_size = min(product_type_count + 2, MAX_TOTAL_PRODUCTS)

    for _ in range(machine_count):
        queue_size = rng.randint(0, max_queue_size)
        total_products += queue_size
        queues.append([rng.randint(0, product_type_count - 2) for _ in range(queue_size)])

    if total_products == 0:
        queues[rng.randrange(machine_count)].append(rng.randint(0, product_type_count - 2))

    return queues


def write_facility(path: Path, machine_count: int, product_type_count: int, rng: random.Random) -> None:
    operation_times = generate_operation_times(rng, machine_count, product_type_count)
    queues = generate_queues(rng, machine_count, product_type_count)

    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8") as output:
        output.write(f"{product_type_count} {machine_count}\n")
        for row in operation_times:
            output.write(" ".join(map(str, row)))
            output.write("\n")
        for queue in queues:
            values = [len(queue), *queue]
            output.write(" ".join(map(str, values)))
            output.write("\n")


def main():
    args = parse_args()
    require_range("N", args.N, 1, MAX_MACHINE_COUNT)
    require_range("M", args.M, 1, MAX_PRODUCT_TYPE_COUNT)

    write_facility(Path(args.output), args.N, args.M, random.Random(args.seed))


if __name__ == "__main__":
    main()
