# Operating-System-Simulation

## Description
This project simulates an Operating System that handles process scheduling, memory management, and mutual exclusion of resources. It reads and executes instructions representing processes, schedules them based on priority, and ensures mutual exclusion for critical resources like file access, user input, and output.

## Features
- **Process Scheduler**: Implements a multilevel feedback model with 4 priority levels for process scheduling.
- **Memory Management**: Fixed memory of 60 words for storing process instructions, variables, and Process Control Blocks (PCBs).
- **Mutual Exclusion**: Uses mutexes to manage access to shared resources (file access, user input, output).
- **Instruction Execution**: Executes program instructions in clock cycles.

## Installation

1. Clone the repository:

    ```bash
    git clone https://github.com/Tonyy131/Operating-System-Simulation.git
    ```

2. Navigate to the project directory:

    ```bash
    cd Project-OS-M2
    ```

3. Build the project:

    ```bash
    make
    ```

4. Run the simulation:

    ```bash
    ./simulator
    ```

## Usage
This project simulates a simple operating system. It:
- Reads and executes program instructions.
- Handles memory allocation and process management.
- Schedules processes using a multilevel feedback queue based on priority.
- Ensures mutual exclusion over shared resources (file access, user input, output).

## License
This project is intended for educational purposes as part of the CSEN 602 course.
