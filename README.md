# Simulated Annealing for TSP

## Overview

The program solves TSP/ATSP instances using Simulated Annealing.

## Features

- support for TSP and ATSP instances
- loading matrix-based instances and TSPLIB files
- initial solution as a random tour or nearest-neighbour based upper bound
- lower bound computed using MST
- geometric and logarithmic cooling schedules
- neighbourhood operators: `swap`, `insert`, `inverse`
- automatic initial temperature estimation
- epoch length dependent on instance size
- stopping by time limit, no-improvement limit, or target relative error
- CSV logging of the algorithm trace

## Configuration

The program is controlled through the `config.txt` file.

Example configuration:

```text
time_limit_ms 900000
min_time_before_target_ms 60000
max_no_improve_epochs 5000
seed 1

use_ub true
use_lb true

cooling geometric
lambda 0.995
neighbourhood inverse

initial_acceptance 0.80
initial_temperature_multiplier 1.0
initial_temperature_samples 2000

epoch_length 0
epoch_factor 20.0

target_error_percent 10.0
log_interval_ms 1000
log_file sa_trace.csv

instance ..\data\tsplib\tsp\pr226.tsp 80369 0
```

Instance line format:

```text
instance <path_to_file> <OPT> <is_ATSP>
```

where:

- `OPT` - optimal or best-known solution value
- `is_ATSP`:
  - `0` - TSP
  - `1` - ATSP

## Main Parameters

| Parameter | Description |
|---|---|
| `time_limit_ms` | maximum runtime for one instance |
| `seed` | random number generator seed |
| `use_ub` | use nearest-neighbour based initial upper bound |
| `use_lb` | compute MST-based lower bound |
| `cooling` | cooling schedule: `geometric` or `logarithmic` |
| `lambda` | geometric cooling coefficient |
| `neighbourhood` | neighbourhood type: `swap`, `insert`, `inverse` |
| `epoch_factor` | epoch length multiplier when `epoch_length = 0` |
| `initial_temperature_multiplier` | multiplier for the estimated initial temperature |
| `max_no_improve_epochs` | stopping limit for epochs without improvement |
| `log_file` | CSV file containing the algorithm trace |



## Output

For each instance, the program prints:

- instance name
- number of vertices
- instance type: TSP or ATSP
- OPT value
- lower bound
- initial cost / upper bound
- best cost found by Simulated Annealing
- relative error against OPT
- runtime
- number of iterations
- initial and final temperature
- stopping reason

Example output:

```text
Instance: ..\data\tsplib\tsp\pr226.tsp
n: 226, type: TSP
OPT: 80369
LB: 68643
Initial cost / UB: 92552
Best SA cost: 81842
Relative error against OPT [%]: 1.833
Time [ms]: 7984.858
```

## Test Instances

The experiments used the following instances:

| Instance | Type | Source | OPT |
|---|---|---|---|
| `sym_14` | TSP | generated | 209 |
| `asym_14` | ATSP | generated | 162 |
| `gr120` | TSP | TSPLIB | 6942 |
| `pr226` | TSP | TSPLIB | 80369 |
| `a280` | TSP | TSPLIB | 2579 |
| `pcb442` | TSP | TSPLIB | 50778 |
| `ftv70` | ATSP | TSPLIB | 1950 |
| `ftv170` | ATSP | TSPLIB | 2755 |
| `rbg323` | ATSP | TSPLIB | 1326 |
| `rbg443` | ATSP | TSPLIB | 2720 |

## Author

Łukasz Cyganiuk
