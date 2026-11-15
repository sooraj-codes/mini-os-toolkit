# Mini OS Toolkit

A single menu-driven C program that demonstrates three core Operating System concepts.
Course: CSC-251L, Operating Systems Lab, BSCS-V (Section D), Sukkur IBA University.

**Team:** Sooraj (Team Lead, Synchronization), Danish Kumar (Deadlock Avoidance), Sajjad Ali (CPU Scheduling)

## Features

| Menu option | Module | What it does |
|---|---|---|
| 1 | CPU Scheduling | FCFS, SJF (non-preemptive and preemptive), Round Robin, Priority (non-preemptive and preemptive). Shows a Gantt chart, waiting time and turnaround time for each process, and the averages. |
| 2 | Deadlock Avoidance | Banker's Algorithm: need matrix, safe-state check with safe sequence, and resource-request handling (grant or deny). |
| 3 | Synchronization | Producer-Consumer (semaphores and mutex) and Dining Philosophers (mutexes with ordered locking). |

## Project structure

```
mini-os-toolkit/
├── main.c                  main menu
├── Makefile
├── README.md
├── scheduling/             scheduling.c / scheduling.h
├── deadlock/               deadlock.c / deadlock.h
└── sync/                   sync.c / sync.h
```

## Requirements

- Linux (Ubuntu recommended), GCC and make
- Also works on Windows (MinGW-w64 GCC), because sync.c has a Windows thread/semaphore layer

Install on Ubuntu: `sudo apt install build-essential`

## How to build and run (Linux)

```bash
cd mini-os-toolkit
make
./mini-os-toolkit
```

Or compile without make:

```bash
gcc -Wall -o mini-os-toolkit main.c scheduling/scheduling.c deadlock/deadlock.c sync/sync.c -lpthread
./mini-os-toolkit
```

Windows (MinGW): `gcc -o mini-os-toolkit.exe main.c scheduling/scheduling.c deadlock/deadlock.c sync/sync.c`

To remove the built program: `make clean`

## How to use

1. Run the program and choose a module from the main menu (1 to 3, or 4 to exit).
2. **Scheduling:** choose *2* to load sample data (or *1* to type your own processes), then pick an algorithm (4 to 9). For Round Robin you will be asked for a time quantum. Lower priority number means higher priority.
3. **Deadlock:** enter the number of processes and resources, then the Allocation matrix, Maximum matrix and Available vector. The program prints the Need matrix and tells you if the system is SAFE. Use option *3* to test a resource request.
4. **Synchronization:** choose *1* for Producer-Consumer or *2* for Dining Philosophers and watch the output.

### Quick Banker's Algorithm test (5 processes, 3 resources)

```
Allocation: 0 1 0 / 2 0 0 / 3 0 2 / 2 1 1 / 0 0 2
Maximum:    7 5 3 / 3 2 2 / 9 0 2 / 2 2 2 / 4 3 3
Available:  3 3 2
```

Expected: SAFE, safe sequence `P1 -> P3 -> P4 -> P0 -> P2`.
Request from P1 of `1 0 2` is granted. After that, a request from P0 of `0 0 2` is denied (it would make the system UNSAFE), and a request from P4 of `1 0 2` is rejected because it exceeds P4's declared need.