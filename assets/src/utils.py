from typing import Dict


def parse_file(filename: str) -> Dict[str, any]:
    data = {}
    solution = []
    in_solution = False

    with open(filename, 'r') as file:
        for line in file:
            line = line.strip()
            if line.startswith("solution="):
                in_solution = True
                continue
            if in_solution:
                if ':' in line:
                    step, positions = line.split(':')
                    positions = positions.strip().split('),')
                    positions = [tuple(map(int, pos.strip().strip('()').split(','))) for pos in positions if pos]
                    solution.append((int(step), positions))
                continue
            if '=' in line:
                key, value = line.split('=', 1)
                key = key.strip()
                value = value.strip()
                if key in ["starts", "goals"]:
                    value = value.split('),')
                    value = [tuple(map(int, v.strip().strip('()').split(','))) for v in value if v]
                data[key] = value
            else:
                continue

    data["solution"] = solution
    return data