import argparse
import random
from typing import Optional
import networkx as nx

resources = ["iron", "gold", "gems", "exp"]


def generate(rooms: int, food: int, seed: int, target_resource: Optional[str]) -> str:
    rng = random.Random(seed)

    G = nx.erdos_renyi_graph(n=(rooms + 1), p=0.2, seed=seed)
    while not nx.is_connected(G):
        seed = rng.randint(-1000000, 1000000)
        G = nx.erdos_renyi_graph(n=(rooms + 1), p=0.2, seed=seed)

    result = f"{rooms}\n"
    for it in range(rooms + 1):
        connected_nodes: list[int] = list(G.neighbors(it))
        adjacent_rooms = ",".join([str(num) for num in connected_nodes])
        result += f"{it} {adjacent_rooms} "
        if it > 0:
            resources_amount = [rng.randint(0, 10), rng.randint(0, 7), rng.randint(0, 5), rng.randint(0, 100)]
            result += " ".join([str(num) for num in resources_amount])
        result += "\n"
    if target_resource is None:
        target_resource = rng.choice(resources)
    result += f"{food} {target_resource}\n"
    return result


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generates a test dungeon for the task")
    parser.add_argument("rooms", type=int, help="Number of rooms in the dungeon")
    parser.add_argument("food", type=int, help="Amount of food provided for the player")
    parser.add_argument("resource", type=str, default=None, nargs="?", help="Target resource for the player")
    parser.add_argument("-s", "--seed", type=int, default=42, help="Seed for the generator")
    parser.add_argument("output", default="dungeon.txt", nargs="?", help="Output file for the generator")

    return parser.parse_args()


def main():
    args = parse_args()
    if args.resource is not None and args.resource not in resources:
        raise Exception(f"Unknown resource: {args.resource}")
    result = generate(args.rooms, args.food, args.seed, args.resource)

    with open(args.output, "w") as f:
        f.write(result)


if __name__ == "__main__":
    main()
