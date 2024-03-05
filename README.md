# ComputerSim README

Overview
This project simulates a simple computer system, consisting of a Central Processing Unit (CPU) and Memory, with a focus on understanding inter-process communication (IPC) and various low-level concepts crucial for proper operating system functionality. These concepts include processor interaction with main memory, processor instruction behavior, registers, stack processing, procedure and system calls, interrupts, memory protection, and I/O. Developed as part of the CS 4348 Operating Systems course at the University of Texas at Dallas.

## Usage:
To run the program, execute the following command in the terminal:
./program_name input_file timer_value

program_name: The name of the compiled program.
input_file: The name of the file containing the program to be executed.
timer_value: An integer specifying the timer constraint for interrupt handling.
    
## Implementation:

### CPU:
    The CPU class encompasses functions for instruction execution, interrupt handling, and memory protection. The project follows an object-oriented approach to design the CPU and      Memory modules.

    Instruction Execution:
    The executeInstruction() function in the CPU class is called at least once within every instruction cycle. It ensures that a timer interrupt has not occurred, reads and             executes the instruction from the Instruction Register (IR).

    Memory Access:
    When reading or writing to/from memory, the CPU ensures proper permissions. The checkPermissions(int address) function is implemented to check if the user program is attempting     to access system memory. If so, and kernel mode is not enabled, an error is indicated, and the program exits gracefully. When writing to memory, the CPU signals the memory          module by writing -1 to the pipe.

    Interrupt Handler:
    The interruptHandler(int code) function handles timer interrupts and system calls. It ensures proper permissions, disables other interrupts to avoid nested execution, saves the     context of the user program on the system stack, jumps to the appropriate address based on the signal received, and enters a loop that executes instruction cycles until the         interrupt is fully handled.

Memory:
    The Memory class contains functions to initialize memory, read from memory, and write to memory.

    Memory Initialization:
    Upon creation, memory is initialized by calling the readInputFile(const char* fileName) function, which reads the user program file into an integer array, skipping invalid          characters and changing the index whenever a '.' is encountered.

    Read Memory:
    The read(int address) function checks that a valid address is being read and returns the value at that address.

    Write Memory:
    The write(int address, int data) function checks if the address is valid, then writes the given data into memory. When writing to memory, a signal of -1 is sent to the memory       process via a pipe, handled within the main function.

Main:
    The main function ensures proper user usage, error handling, forks the processes, initializes pipes used for IPC, and manages instruction cycles until program execution. A          signal code of -5 signals to the memory process that it is free to close the pipes and exit, ensuring a graceful exit anytime the CPU exits.
