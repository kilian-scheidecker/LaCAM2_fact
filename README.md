LaCAM2_fact
---
[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE)
[![CI](https://github.com/Kei18/lacam2/actions/workflows/ci.yml/badge.svg)](https://github.com/Kei18/lacam2/actions/workflows/ci.yml)

This is a factorized version of the LaCAM2 algorithm from Keisuke Okumura.

Do you want the power?
LaCAM* could be the answer.

The original work is from code repository of the paper ["Improving LaCAM for Scalable Eventually Optimal Multi-Agent Pathfinding"](https://kei18.github.io/lacam2/) (IJCAI-23), extended from from [the LaCAM repo](https://kei18.github.io/lacam/) presented at AAAI-23.

## Building

To build the project you need is [CMake](https://cmake.org/) (≥v3.16). The code is written in C++(17).

First, clone this repo with submodules.

```sh
git clone https://github.com/idsc-frazzoli/LaCAM2_fact.git
```

Depending on the configuration of your environnement, you might also need other linux libraries such as qt-5-default. To install the required packages, run the following command.

```sh
sudo apt-get install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
```

Then dowload the required submodules. This project uses two submodules: [argparse](https://github.com/p-ranav/argparse) by P-Ranav and [easy_profiler](https://github.com/yse/easy_profiler) by yse.

```sh
cd LaCAM2_fact
git submodules init
git submodules update
```

Finally, you can build the project.

```sh
cmake -B build && make -C build -j4
```


## Usage

Basic run with 50 agents on the 'random-32-32-20' map. The important arguments are :
-i : the path to the .scen file specifying the start and goal location for every agent.
-m : the path to the .map file of the given problem.
-N : the number of agents.
-v : the verbosity level. for -v 0,  the code will not provide any output to the terminal.

Try running the following command :

```sh
> build/main -i assets/maps/random-32-32-20/other_scenes/random-32-32-20-50.scen -m assets/maps/random-32-32-20/random-32-32-20.map -N 50 -v 1
solved: 1ms     makespan: 47 (lb=47, ub=1)      sum_of_costs: 1297 (lb=1098, ub=1.19)   sum_of_loss: 1198 (lb=1098, ub=1.1)
```

There are other arguments you can specify in order to use all the feature of LaCAM2_fact :
-f  : the heuristic used in the factorization. '-f no' will use standard LaCAM2.
-mt : to enable multi-threading.

You can find details of all parameters with:
```sh
build/main --help
```

Note that some parameters are only compatible with the standard version and some others only with the factorized verison of LaCAM2.

## Visualizer

This repository is compatible with [@Kei18/mapf-visualizer](https://github.com/kei18/mapf-visualizer).

## Data visualization

TODO

## Code Profiling

You can take advantage of the [Easy Profiler library](https://github.com/yse/easy_profiler) in order to analyze the code and dive into the internals of the algorithms.

To use the profiling, the variable ENABLE_PROFILING needs to be defined. Be aware that toggling the profiler may affect performances.

To toggle the profiling mode, you need to build the project accordingly by setting ENABLE_PROFILING=ON.

```sh
cmake -D ENABLE_PROFILING=ON -B build && make -C build -j4
```

Changing the definition of ENABLE_PROFILING, requires you to rebuild the project. 
At every run, the collected data will be stored in 'code_profiling/profile.prof'.

This file can then be vizualised by using the Easy Profiler Visualizer. There should be an executable called 'profiler_gui' in the build directory of the Easy Profiler. You can use this to visualize everything in detail.

```sh
./build/third_party/easy_profiler/bin/profiler_gui
```

Once you opened the visualizer, you can use the folder icon at the top left to open the 'profile.prof' file.

To stop profiling, clean build the project again using the instructions in the 'Building' section.


## Notes

- The grid maps and scenarios in `assets/maps/` are from [MAPF benchmarks](https://movingai.com/benchmarks/mapf.html).
- `tests/` is not comprehensive. It was used in early developments.
- Auto formatting (clang-format) when committing:

```sh
git config core.hooksPath .githooks && chmod a+x .githooks/pre-commit
```

LaCAM* variants are available in [tags](https://github.com/Kei18/lacam2/tags).


## Licence

This software is released under the MIT License, see [LICENSE.txt](LICENCE.txt).
