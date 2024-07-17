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
cmake -B build && make -C build -j
```


## Usage

no optimization (random starts/goals):

```sh
> build/main -v 1 -m assets/random-32-32-20.map -N 400
solved: 31ms    makespan: 112 (lb=58, ub=1.94)  sum_of_costs: 31373 (lb=9217, ub=3.41)  sum_of_loss: 26001 (lb=9217, ub=2.83)

# with the MAPF visualizer mentioned below
> mapf-visualizer map/random-32-32-20.map build/result.txt
```

![](assets/demo-random-32-32-20_400agents.gif)

makespan optimization:

```sh
> build/main -m assets/loop.map -i assets/loop.scen -N 3 -v 1 --objective 1
solved: 8ms     makespan: 10 (lb=2, ub=5)       sum_of_costs: 21 (lb=5, ub=4.2) sum_of_loss: 21 (lb=5, ub=4.2)
```

sum-of-loss optimization:

```sh
> build/main -m assets/loop.map -i assets/loop.scen -N 3 -v 2 --objective 2
solved: 1ms     makespan: 11 (lb=2, ub=5.5)     sum_of_costs: 15 (lb=5, ub=3)   sum_of_loss: 15 (lb=5, ub=3)
```

You can find details of all parameters with:
```sh
build/main --help
```

## Visualizer

This repository is compatible with [@Kei18/mapf-visualizer](https://github.com/kei18/mapf-visualizer).

## Data visualization

TODO

## Debug

You can take advantage of the [Easy Profiler library](https://github.com/yse/easy_profiler) in order to debug the code and dive into the internals of the algorithms. To use the library you need to build the profiler as well.

```sh
cd third_party/easy_profiler
cmake -B build && make -C build
```

Note: you might need to install additionnal packages in order to build the profiler. Most likely qt5 :

```sh
sudo apt-get install qtbase5-dev
```


Once the profiler has been built, you can use the optional argument '-d yes' in the command line when solving an instance. This will set the code into pofiling mode and store the data in a file called profile.prof.

This file can then be vizualised by using the Easy Profiler Visualizer. There should be an executable called 'profiler_gui' in the build directory of the Easy Profiler. You can use this to visualize everything in detail.

```sh
cd 
./third_party/easy_profiler/build/bin/profiler_gui
```

Once you opened the visualizer, you can use the folder icon at the top left to open the 'profile.prof' file.

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
