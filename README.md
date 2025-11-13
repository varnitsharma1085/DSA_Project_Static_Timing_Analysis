# Static Timing Analysis (STA) Tool in C++

This project is a command-line Static Timing Analysis (STA) tool built in C++. It is designed as an educational tool to demonstrate key Object-Oriented Programming (OOP) principles, graph theory (DAGs), and fundamental Data Structures and Algorithms (DSA) as applied to digital circuit design.

The tool parses a text-based circuit netlist and a gate delay library. It then builds a graph representation of the circuit to perform timing analysis, identify the critical path, and report any timing violations.

##  Features

  * **Circuit Parsing:** Reads simple text-based netlists (`.txt`) and gate delay libraries.
  * **Graph-Based Architecture:** Builds a **Directed Acyclic Graph (DAG)** to represent the circuit, where `Node` objects represent wires and `Gate` objects represent the connections and delays.
  * **Object-Oriented Design:** Uses **polymorphism** and a **Factory Pattern** to manage a wide variety of logic gates (AND, OR, NOT, XOR, Adders, MUXs).
  * **Core Timing Analysis:**
      * **Forward Propagation:** Calculates **Arrival Times** using a **Topological Sort**.
      * **Backward Propagation:** Calculates **Required Times** using a reverse topological sort.
      * **Slack Calculation:** Determines the timing margin (`Slack = RequiredTime - ArrivalTime`) for every node.
  * **Critical Path Identification:** Uses **Depth-First Search (DFS)** to find all paths and identifies the critical path(s) (those with the worst/lowest slack).
  * **Report Generation:** Outputs a detailed timing report to the `reports/` directory.

-----

##  Project Structure

```
Static-Timing-Analysis/
├── src/                  # Source code (.h/.cpp files)
│   ├── main.cpp          # Main program entry point
│   ├── Circuit.h     # Manages the overall circuit graph
│   ├── Node.h        # Represents wires/connection points
│   ├── Gate.h        # Base gate classes and specific implementations
│   └── TimingAnalyzer.h # Core STA algorithm engine
├── examples/             # Example circuit files
│   ├── simple_circuit.txt
│   ├── complex_circuit.txt
│   └── adder_circuit.txt
├── delays/               # Gate delay configuration files
│   └── gate_delays.txt
├── reports/              # Generated timing reports (created at runtime)
├── obj/                  # Object files (created during build)
├── bin/                  # Executable files (created during build)
└── README.md             # This file

```

-----

## How to Build and Run


### Prerequisites

  * A C++17 compatible compiler (e.g., `g++` or `clang`)
  * `make`

### Build Instructions

1.  **Clone the repository:**

    ```bash
    git clone https://github.com/varnitsharma1085/DSA_Project_Static_Timing_Analysis.git
    cd Static-Timing-Analysis
    ```

2.  **Build the executable:**
     Run the build.bat file.

### Running the Analyzer
Simply run the sta.exe file created upon running build.bat file.
In the "reports" folder the result will be saved of the example you have put in the main.cpp file in int main().


Alternatively
When all the required files are in place, simply open main.cpp using vs code. Change the path of the circuit you want to analyze in the main function, execute the main.cpp file using any g++ compiler.

## How It Works: Conceptual Flow

The entire analysis is a multi-step process orchestrated by the `TimingAnalyzer`.

1.  **Parsing & Graph Building:**

      * The `Circuit` class reads the `examples/` and `delays/` files.
      * It creates `Node` (wire) and `Gate` objects.
      * It builds the graph by linking nodes to gates via `fanin` and `fanout` pointers.

2.  **Step 1: Forward Propagation (Arrival Time)**

      * A **topological sort** (Kahn's algorithm) is used to process nodes in the correct order (inputs first).
      * The `arrivalTime` (AT) is calculated for each node:
      * **Formula:** `AT_output = max(all_input_ATs) + gate_delay`
      * This finds the *longest* time it takes for a signal to reach each node from the start.

3.  **Step 2: Backward Propagation (Required Time)**

      * A **reverse topological sort** is used, starting from the primary outputs.
      * The `requiredTime` (RT) is set to the `CLOCK_PERIOD` for all outputs.
      * The analyzer works backward, calculating the *latest* time a signal must arrive at a node to meet the clock deadline.
      * **Formula:** `RT_input = min(all_output_RTs) - gate_delay`

4.  **Step 3: Slack Calculation**

      * For every node in the graph, the slack (timing margin) is calculated.
      * **Formula:** `Slack = RequiredTime - ArrivalTime`
      * **`Slack > 0`**: The signal arrived *before* it was needed (Good).
      * **`Slack < 0`**: The signal arrived *after* it was needed (**Timing Violation**).

5.  **Step 4: Critical Path Finding**

      * A **Depth-First Search (DFS)** is used to find all possible paths from primary inputs to primary outputs.
      * The **critical path** is the path with the lowest (most negative) slack. This is the slowest path in the circuit and determines its maximum operating speed.

-----

## Core C++ Components

The project is built on four key classes:

  * **`Node.h/cpp`**
    Represents a wire or connection point. This class is the "scorecard" that stores all timing data (`arrivalTime`, `requiredTime`, `slack`) and holds the graph structure (`fanin` and `fanout` pointers).

  * **`Gate.h/cpp`**
    Uses **inheritance** and **polymorphism** to define a base `Gate` class and all specific implementations (`ANDGate`, `NOTGate`, etc.). Each gate knows its own logic (`evaluate()`) and delay. A `GateFactory` is used to create the correct gate object from a string (e.g., "AND").

  * **`Circuit.h/cpp`**
    The main "container" for the circuit. It holds a `map` of all `Node` objects (for fast lookup by name) and a `vector` of all `Gate` objects. It is responsible for parsing the input files and building the graph.

  * **`TimingAnalyzer.h/cpp`**
    The "brain" of the operation. This class takes a `Circuit` object and orchestrates all the key algorithms: the forward/backward traversals, slack calculation, path finding, and final report generation.

-----

##  Input File Formats

### Circuit File (`examples/*.txt`)

Defines the circuit structure using simple keywords:

```
# Comments start with a hash
CLOCK_PERIOD <time_in_ns>
INPUT <list_of_input_nodes>
OUTPUT <list_of_output_nodes>
GATE <gate_type> <gate_name> <output_node> <list_of_input_nodes>

# Example:
CLOCK_PERIOD 1.0
INPUT A B C
OUTPUT F
GATE AND G1 temp1 A B
GATE OR G2 F temp1 C
```

### Delay File (`delays/gate_delays.txt`)

Defines the delay for each gate type.

```
# Comments start with a hash
<gate_type> <delay_in_ns>

# Example:
AND 0.1
OR 0.12
NOT 0.05
```

-----

## How to Extend the Project

Adding a new logic gate is simple thanks to the object-oriented design:

1.  **Create the Gate Class:**

      * Create a new class (e.g., `AND3Gate`) in `Gate.h/cpp` that inherits from the base `Gate` class.
      * Implement its `evaluate()` function for its specific logic.

2.  **Add to the Factory:**

      * In `Gate.cpp`, add your new gate to the `GateFactory::createGate` function:
        ```cpp
        if (type == "AND3") return new AND3Gate(...);
        ```

3.  **Add the Delay:**

      * Open `delays/gate_delays.txt` and add the delay for your new gate:
        ```
        AND3 0.15
        ```

You can now use `AND3` in your circuit files, and the analyzer will automatically support it.
