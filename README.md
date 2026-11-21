# Microrécif

An interactive 2D marine ecosystem simulation built with C++17, GTKmm 4 and Cairo.
Algae spawn and age, corals feed and reproduce, and scavengers consume dead corals.

![Reef evolution after 100 simulation steps](docs/illustrations/recif.png)

## Features

- Continuous and step-by-step simulation.
- Coral growth and reproduction with collision detection.
- Assignment of dead corals to the nearest available scavengers, with one
  scavenger per target.
- Optional random algae generation.
- Loading and saving the world state as plain text files.
- Resizable interface, population counters and keyboard shortcuts.

## Running the simulation

From the project root:

```sh
make
make demo
```

The demo starts paused. Enable **Naissance d'algue** (algae generation), then click
**Start**. Living corals are blue, dead corals are black, algae are green and
scavengers are red. An ecosystem may die out as organisms reach their maximum
lifespan.

To load another world or start with an empty window:

```sh
./projet tests/t41.txt
./projet
```

| Control | Action | Shortcut |
| --- | --- | --- |
| Start / Stop | Run or pause the simulation | `s` |
| Step | Advance one step while paused | `1` |
| Open | Load a simulation file | — |
| Save | Save the current state | — |
| Exit | Close the application | `q` |

Open and Save pause the simulation. The interval between steps is set by
`speed_time_ms` in `gui.cc`: 10 ms by default, or approximately 100 steps per
second depending on the workload. After changing this value, run `make` and
restart the application.

## Project structure

| Files | Purpose |
| --- | --- |
| `projet.cc` | Application entry point |
| `gui.*` | GTK interface and simulation timer |
| `simulation.*` | World updates, loading and saving |
| `lifeform.*` | Algae, corals and scavengers |
| `shape.*` | Geometry and collision detection |
| `graphic.*`, `graphic_gui.h` | Cairo drawing primitives |
| `constantes.h`, `message.*` | Model parameters and diagnostics |
| `examples/` | Demo world |
| `tests/` | Scenarios and automated tests |
| `docs/` | Architecture, file format and illustrations |

Additional documentation is available in French:

- [Architecture and model rules](docs/architecture.md)
- [Simulation file format](docs/file-format.md)
- [Test organization](tests/README.md)

## Authors

Antoine Bernadat and Timothy Biersohn.

Originally developed as a second-year undergraduate object-oriented programming
project, then extended with automated tests and exclusive scavenger target
assignment.
