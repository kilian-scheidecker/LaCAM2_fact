LaCAM2_fact
---
[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE)
[![CI](https://github.com/Kei18/lacam2/actions/workflows/ci.yml/badge.svg)](https://github.com/Kei18/lacam2/actions/workflows/ci.yml)

This is a factorized version of the LaCAM2 algorithm from Keisuke Okumura.

Do you want the power?
LaCAM* could be the answer.

The original work is from code repository of the paper ["Improving LaCAM for Scalable Eventually Optimal Multi-Agent Pathfinding"](https://kei18.github.io/lacam2/) (IJCAI-23), extended from from [the LaCAM repo](https://kei18.github.io/lacam/) presented at AAAI-23.

## Building

All you need is [CMake](https://cmake.org/) (≥v3.16). The code is written in C++(17).

First, clone this repo with submodules.

```sh
git clone https://github.com/idsc-frazzoli/LaCAM2_fact.git && cd LaCAM2_fact
git submodules init
git submodules update
```

Then, build the project.

```sh
cmake -B build && make -C build -j4
```


## Usage

no optimization (random starts/goals):

```sh
> build/main -i assets/random-32-32-20/other_scenes/random-32-32-20-50.scen -m assets/random-32-32-20/random-32-32-20.map -N 50 -v 1 -f no -mt no
solved: 1ms     makespan: 47 (lb=47, ub=1)      sum_of_costs: 1297 (lb=1098, ub=1.19)   sum_of_loss: 1198 (lb=1098, ub=1.1)

# with the MAPF visualizer mentioned below
> mapf-visualizer map/random-32-32-20.map build/result.txt
```

![](assets/demo-random-32-32-20_400agents.gif)

makespan optimization:

```sh
> build/main -i assets/random-32-32-20/other_scenes/random-32-32-20-50.scen -m assets/random-32-32-20/random-32-32-20.map -N 50 -v 1 -f no --objective 1
solved: 1ms     makespan: 47 (lb=47, ub=1)      sum_of_costs: 1297 (lb=1098, ub=1.19)   sum_of_loss: 1198 (lb=1098, ub=1.1)
```

sum-of-loss optimization:

```sh
> build/main -i assets/random-32-32-20/other_scenes/random-32-32-20-50.scen -m assets/random-32-32-20/random-32-32-20.map -N 50 -v 1 -f no --objective 2
solved: 10258ms makespan: 47 (lb=47, ub=1)      sum_of_costs: 1252 (lb=1098, ub=1.15)   sum_of_loss: 1192 (lb=1098, ub=1.09)
```

You can find details of all parameters with:
```sh
build/main --help
```

## Visualizer

This repository is compatible with [@Kei18/mapf-visualizer](https://github.com/kei18/mapf-visualizer).

## Data visualization

TODO

## Code Profiling

You can take advantage of the [Easy Profiler library](https://github.com/yse/easy_profiler) in order to analyze the code and dive into the internals of the algorithms.

To use the profiling, the variable ENABLE_PROFILING needs to be defined in utils.hpp (line 34). De-comment the line to define the variable and set the code in profiling mdoe. Be aware that toggling the profiler may affect performances.

Once you changed the definition of ENABLE_PROFILING, you need to rebuild the project. 

```sh
make -C build -j4
```

At every run, the collected data will be stored in the file called 'profile.prof'.

This file can then be vizualised by using the Easy Profiler Visualizer. There should be an executable called 'profiler_gui' in the build directory of the Easy Profiler. You can use this to visualize everything in detail.

```sh
./build/third_party/easy_profiler/bin/profiler_gui
```

Once you opened the visualizer, you can use the folder icon at the top left to open the 'profile.prof' file.

To stop profiling, clean build the project again using the instructions in the 'Building' section.


## Notes

- The grid maps and scenarios in `assets/` are from [MAPF benchmarks](https://movingai.com/benchmarks/mapf.html).
- `tests/` is not comprehensive. It was used in early developments.
- Auto formatting (clang-format) when committing:

```sh
git config core.hooksPath .githooks && chmod a+x .githooks/pre-commit
```

LaCAM* variants are available in [tags](https://github.com/Kei18/lacam2/tags).


## Licence

This software is released under the MIT License, see [LICENSE.txt](LICENCE.txt).
