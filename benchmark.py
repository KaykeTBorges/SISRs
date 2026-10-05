#!/usr/bin/env python3
import csv
import re
import subprocess
import sys
from decimal import Decimal
from pathlib import Path


INSTANCES = (
    "X-n101-k25",
    "X-n176-k26",
    "X-n242-k48",
    "X-n251-k28",
    "X-n401-k29",
    "X-n469-k138",
    "X-n627-k43",
    "X-n801-k40",
    "X-n1001-k43",
)
RUNS_PER_INSTANCE = 10
ROOT = Path(__file__).resolve().parent
EXECUTABLE = ROOT / "leitor"
RESULTS_FILE = ROOT / "resultados.csv"
ROUTES_FILE = ROOT / "rotas.csv"
MEANS_FILE = ROOT / "medias_por_instancia.csv"
NUMBER = r"([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?)"
ROUTE_PATTERN = re.compile(
    rf"^Tour\s+(\d+):\s*(.*?)\s*\|\s*custo\s*=\s*{NUMBER}\s*$",
    re.MULTILINE,
)


def parse_result(output: str) -> tuple[str, str, str, list[tuple[str, str, str]]]:
    try:
        final_section = output.split("--- Solucao final ---", 1)[1]
        final_section = final_section.split("--- Rotas finais ---", 1)[0]
        routes_section = output.split("--- Rotas finais ---", 1)[1]
        routes_section = routes_section.split("\nClientes ausentes:", 1)[0]
    except IndexError as error:
        raise ValueError("a saída não contém as seções finais esperadas") from error

    cost_match = re.search(rf"^Custo:\s*{NUMBER}\s*$", final_section, re.MULTILINE)
    vehicles_match = re.search(
        r"^Numero de veiculos:\s*(\d+)\s*$", final_section, re.MULTILINE
    )
    time_match = re.search(
        rf"^Tempo \(segundos\):\s*{NUMBER}\s*$", final_section, re.MULTILINE
    )
    routes = ROUTE_PATTERN.findall(routes_section)

    if not cost_match or not vehicles_match or not time_match:
        raise ValueError("não foi possível extrair custo, veículos e tempo da saída")
    if len(routes) != int(vehicles_match.group(1)):
        raise ValueError("o número de rotas extraídas não corresponde aos veículos impressos")

    return cost_match.group(1), vehicles_match.group(1), time_match.group(1), routes


def main() -> int:
    if not EXECUTABLE.is_file():
        print(f"Executável não encontrado: {EXECUTABLE}\nCompile com: make", file=sys.stderr)
        return 1

    missing_instances = [
        name for name in INSTANCES if not (ROOT / "X" / f"{name}.vrp").is_file()
    ]
    if missing_instances:
        print("Instâncias não encontradas: " + ", ".join(missing_instances), file=sys.stderr)
        return 1

    measurements = {
        instance: {"cost": [], "vehicles": [], "time": []}
        for instance in INSTANCES
    }

    with (
        RESULTS_FILE.open("w", newline="", encoding="utf-8") as csv_file,
        ROUTES_FILE.open("w", newline="", encoding="utf-8") as routes_file,
        MEANS_FILE.open("w", newline="", encoding="utf-8") as means_file,
    ):
        writer = csv.writer(csv_file)
        routes_writer = csv.writer(routes_file)
        means_writer = csv.writer(means_file)
        writer.writerow(("instance", "run", "seed", "cost", "vehicles", "time"))
        routes_writer.writerow(("instance", "run", "seed", "tour", "route", "cost"))
        means_writer.writerow(("instance", "runs", "mean_cost", "mean_vehicles", "mean_time"))

        for instance in INSTANCES:
            instance_path = ROOT / "X" / f"{instance}.vrp"

            for run in range(1, RUNS_PER_INSTANCE + 1):
                seed = run
                print(f"[{instance}] execução {run}/{RUNS_PER_INSTANCE} - seed {seed}", flush=True)

                completed = subprocess.run(
                    [str(EXECUTABLE), str(instance_path), str(seed)],
                    cwd=ROOT,
                    capture_output=True,
                    text=True,
                    check=False,
                )
                if completed.returncode != 0:
                    print(completed.stdout, end="", file=sys.stderr)
                    print(completed.stderr, end="", file=sys.stderr)
                    print(
                        f"Falha ao executar {instance} com seed {seed} "
                        f"(código {completed.returncode}).",
                        file=sys.stderr,
                    )
                    return 1

                try:
                    cost, vehicles, elapsed, routes = parse_result(completed.stdout)
                except ValueError as error:
                    print(f"Erro na saída de {instance}, seed {seed}: {error}", file=sys.stderr)
                    return 1

                writer.writerow((instance, run, seed, cost, vehicles, elapsed))
                for tour, route, route_cost in routes:
                    routes_writer.writerow((instance, run, seed, tour, route, route_cost))
                csv_file.flush()
                routes_file.flush()
                measurements[instance]["cost"].append(Decimal(cost))
                measurements[instance]["vehicles"].append(Decimal(vehicles))
                measurements[instance]["time"].append(Decimal(elapsed))
                print(f"Custo: {cost}")
                print(f"Veículos: {vehicles}")
                print(f"Tempo: {elapsed} segundos\n", flush=True)

        print("Médias por instância:")
        for instance in INSTANCES:
            values = measurements[instance]
            run_count = len(values["cost"])
            mean_cost = sum(values["cost"], Decimal(0)) / run_count
            mean_vehicles = sum(values["vehicles"], Decimal(0)) / run_count
            mean_time = sum(values["time"], Decimal(0)) / run_count
            means_writer.writerow(
                (instance, run_count, mean_cost, mean_vehicles, mean_time)
            )
            print(
                f"{instance}: custo={mean_cost}, veículos={mean_vehicles}, "
                f"tempo={mean_time} segundos"
            )

    print(
        "Concluído: 90 execuções registradas em resultados.csv, "
        "rotas.csv e medias_por_instancia.csv."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
