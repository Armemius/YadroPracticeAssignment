import argparse
from dataclasses import dataclass, field
from pathlib import Path
import matplotlib.animation as animation
import matplotlib.pyplot as plt
import networkx as nx

RESOURCES = ("iron", "gold", "gems", "exp")
MIN_FPS = 1.0
MAX_DURATION_SECONDS = 7.5


@dataclass
class Room:
    idx: int
    adjacent: list[int]
    resources: dict[str, int | str]


@dataclass
class Frame:
    title: str
    current_room: int = 0
    last_edge: tuple[int, int] | None = None
    visited: set[int] = field(default_factory=lambda: {0})
    rooms: dict[int, Room] = field(default_factory=dict)
    log_tail: list[str] = field(default_factory=list)


@dataclass(frozen=True)
class VisualStyle:
    node_size: int
    node_border_width: float
    default_edge_width: float
    active_edge_width: float
    title_font_size: float
    index_font_size: float
    resource_font_size: float
    index_offset: float
    resource_offset: float


def parse_input(path: Path) -> tuple[dict[int, Room], str]:
    lines = path.read_text(encoding="utf-8").splitlines()
    if not lines:
        raise ValueError(f"{path} is empty")

    room_count = int(lines[0])
    room_lines = lines[1:room_count + 2]
    player_line = lines[room_count + 2] if len(lines) > room_count + 2 else ""

    rooms: dict[int, Room] = {}
    for line in room_lines:
        parts = line.split()
        if len(parts) == 2:
            parts += ["0", "0", "0", "0"]
        if len(parts) != 6:
            raise ValueError(f"invalid room line: {line}")

        idx = int(parts[0])
        adjacent = [int(room) for room in parts[1].split(",") if room]
        resources: dict[str, str | int] = {name: int(value) for name, value in zip(RESOURCES, parts[2:])}
        rooms[idx] = Room(idx=idx, adjacent=adjacent, resources=resources)

    return rooms, player_line


def parse_output(path: Path) -> list[str]:
    return [line.strip() for line in path.read_text(encoding="utf-8").splitlines() if line.strip()]


def build_graph(rooms: dict[int, Room]) -> nx.Graph:
    graph = nx.Graph()
    for room in rooms.values():
        graph.add_node(room.idx)
        for adjacent in room.adjacent:
            graph.add_edge(room.idx, adjacent)
    return graph


def apply_state(line: str, rooms: dict[int, Room]) -> int:
    parts = line.split()
    if len(parts) != 6 or parts[0] != "state":
        raise ValueError(f"invalid state line: {line}")

    room_idx = int(parts[1])
    room = rooms[room_idx]
    for name, value in zip(RESOURCES, parts[2:]):
        room.resources[name] = value if value == "_" else int(value)
    return room_idx


def apply_collect(resource: str, current_room: int, rooms: dict[int, Room]) -> None:
    if resource not in RESOURCES:
        raise ValueError(f"invalid resource name: {resource}")
    rooms[current_room].resources[resource] = "_"


def make_frames(rooms: dict[int, Room], output_lines: list[str]) -> list[Frame]:
    current_room = 0
    visited = {0}
    log_tail: list[str] = []
    frames = [
        Frame(
            title="Start at room 0",
            current_room=current_room,
            visited=set(visited),
            rooms=copy_rooms(rooms),
        )
    ]

    for line in output_lines:
        parts = line.split()
        if not parts:
            continue

        if parts[0] == "go" and len(parts) == 2:
            target = int(parts[1])
            source = current_room
            log_tail.append(line)
            log_tail = log_tail[-8:]
            current_room = target
            visited.add(target)
            frames.append(
                Frame(
                    title=f"Go to room {target}",
                    current_room=current_room,
                    last_edge=tuple(sorted((source, target))),  # type: ignore
                    visited=set(visited),
                    rooms=copy_rooms(rooms),
                    log_tail=list(log_tail),
                )
            )
        elif parts[0] == "collect" and len(parts) == 2:
            log_tail.append(line)
            log_tail = log_tail[-8:]
            apply_collect(parts[1], current_room, rooms)
            frames.append(
                Frame(
                    title=f"Collect {parts[1]}",
                    current_room=current_room,
                    visited=set(visited),
                    rooms=copy_rooms(rooms),
                    log_tail=list(log_tail),
                )
            )
        elif parts[0] == "state":
            current_room = apply_state(line, rooms)
            visited.add(current_room)
        elif parts[0] == "result":
            continue
        else:
            continue

    return frames


def copy_rooms(rooms: dict[int, Room]) -> dict[int, Room]:
    return {
        idx: Room(idx=room.idx, adjacent=list(room.adjacent), resources=dict(room.resources))
        for idx, room in rooms.items()
    }


def room_label(room: Room) -> str:
    return " ".join(str(room.resources[name]) for name in RESOURCES)


def visual_style(node_count: int) -> VisualStyle:
    if node_count <= 0:
        raise ValueError("graph has no nodes")

    linear_scale = min(1.0, max(0.34, (12 / node_count) ** 0.45))
    area_scale = linear_scale * linear_scale
    return VisualStyle(
        node_size=max(260, round(2400 * area_scale)),
        node_border_width=max(0.6, 1.5 * linear_scale),
        default_edge_width=max(0.55, 1.4 * linear_scale),
        active_edge_width=max(1.4, 3.5 * linear_scale),
        title_font_size=max(11, 16 * linear_scale),
        index_font_size=max(5.5, 11 * linear_scale),
        resource_font_size=max(4.5, 9 * linear_scale),
        index_offset=max(3, 8 * linear_scale),
        resource_offset=-max(4, 9 * linear_scale),
    )


def draw_frame(
    frame: Frame,
    graph: nx.Graph,
    positions: dict[int, tuple[float, float]],
    style: VisualStyle,
) -> None:
    plt.clf()
    figure = plt.gcf()
    figure.set_size_inches(11, 7)

    graph_axis = plt.gca()
    graph_axis.set_title(frame.title, fontsize=style.title_font_size)
    graph_axis.axis("off")

    node_colors = []
    for node in graph.nodes:
        if node == frame.current_room:
            node_colors.append("#f59e0b")
        elif node == 0:
            node_colors.append("#60a5fa")
        elif node in frame.visited:
            node_colors.append("#86efac")
        else:
            node_colors.append("#d1d5db")

    edge_colors = []
    edge_widths = []
    for source, target in graph.edges:
        edge = tuple(sorted((source, target)))
        if frame.last_edge == edge:
            edge_colors.append("#ef4444")
            edge_widths.append(style.active_edge_width)
        else:
            edge_colors.append("#9ca3af")
            edge_widths.append(style.default_edge_width)

    nx.draw_networkx_edges(
        graph,
        positions,
        ax=graph_axis,
        edge_color=edge_colors,
        width=edge_widths,
    )
    nx.draw_networkx_nodes(
        graph,
        positions,
        ax=graph_axis,
        node_color=node_colors,
        node_size=style.node_size,
        edgecolors="#111827",
        linewidths=style.node_border_width,
    )
    for idx, (x, y) in positions.items():
        graph_axis.annotate(
            str(idx),
            xy=(x, y),
            xytext=(0, style.index_offset),
            textcoords="offset points",
            ha="center",
            va="center",
            fontsize=style.index_font_size,
            fontweight="bold",
        )
        graph_axis.annotate(
            room_label(frame.rooms[idx]),
            xy=(x, y),
            xytext=(0, style.resource_offset),
            textcoords="offset points",
            ha="center",
            va="center",
            fontsize=style.resource_font_size,
        )
    plt.tight_layout()


def effective_fps(requested_fps: float, frame_count: int) -> float:
    if requested_fps <= 0:
        raise ValueError("fps must be positive")
    if frame_count <= 0:
        raise ValueError("simulation has no frames")

    duration_limited_fps = frame_count / MAX_DURATION_SECONDS
    return max(MIN_FPS, requested_fps, duration_limited_fps)


def render_gif(input_path: Path, output_path: Path, gif_path: Path) -> float:
    rooms, _ = parse_input(input_path)
    output_lines = parse_output(output_path)
    graph = build_graph(rooms)
    positions = nx.spring_layout(graph, seed=7)
    style = visual_style(graph.number_of_nodes())
    frames = make_frames(rooms, output_lines)
    fps = effective_fps(MIN_FPS, len(frames))

    figure = plt.figure(figsize=(11, 7))

    def update(index: int) -> None:
        draw_frame(frames[index], graph, positions, style)

    gif_path.parent.mkdir(parents=True, exist_ok=True)
    interval = int(1000 / fps)
    gif = animation.FuncAnimation(figure, update, frames=len(frames), interval=interval, repeat=True)
    gif.save(str(gif_path), writer=animation.PillowWriter(fps=fps))  # type: ignore
    plt.close(figure)
    return fps


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Create a GIF from a task input file and a results file")
    parser.add_argument("input", type=Path, help="task input file, for example in.txt")
    parser.add_argument(
        "output",
        type=Path,
        nargs="?",
        default=Path("result.txt"),
        help="bot output file, defaults to result.txt",
    )
    parser.add_argument(
        "-o",
        "--gif",
        type=Path,
        default=Path("playback.gif"),
        help="GIF path to write, defaults to playback.gif",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    render_gif(args.input, args.output, args.gif)
    print(f"GIF saved to {args.gif}")


if __name__ == "__main__":
    main()
