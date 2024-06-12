# ComputerSim README

Overview
This project simulates a simple computer system, consisting of a Central Processing Unit (CPU) and Memory, with a focus on understanding inter-process communication (IPC) and various low-level concepts crucial for proper operating system functionality. These concepts include processor interaction with main memory, processor instruction behavior, registers, stack processing, procedure and system calls, interrupts, memory protection, and I/O. Developed as part of the CS 4348 Operating Systems course at the University of Texas at Dallas.

## Usage
To run the program, execute the following command in the terminal:    

./program_name input_file timer_value

program_name: The name of the compiled program.

input_file: The name of the file containing the program to be executed.

timer_value: An integer specifying the timer constraint for interrupt handling.
    
## Implementation

### CPU:
The CPU class encompasses functions for instruction execution, interrupt handling, and memory protection. The project follows an object-oriented approach to design the CPU and      Memory modules.

#### Instruction Execution:
The executeInstruction() function in the CPU class is called at least once within every instruction cycle. It ensures that a timer interrupt has not occurred, reads and             executes the instruction from the Instruction Register (IR).

#### Memory Access:
When reading or writing to/from memory, the CPU ensures proper permissions. The checkPermissions(int address) function is implemented to check if the user program is attempting     to access system memory. If so, and kernel mode is not enabled, an error is indicated, and the program exits gracefully. When writing to memory, the CPU signals the memory          module by writing -1 to the pipe.

#### Interrupt Handler:
The interruptHandler(int code) function handles timer interrupts and system calls. It ensures proper permissions, disables other interrupts to avoid nested execution, saves the     context of the user program on the system stack, jumps to the appropriate address based on the signal received, and enters a loop that executes instruction cycles until the         interrupt is fully handled.

### Memory
The Memory class contains functions to initialize memory, read from memory, and write to memory.

#### Memory Initialization:
Upon creation, memory is initialized by calling the readInputFile(const char* fileName) function, which reads the user program file into an integer array, skipping invalid          characters and changing the index whenever a '.' is encountered.

#### Read Memory:
The read(int address) function checks that a valid address is being read and returns the value at that address.

#### Write Memory:
The write(int address, int data) function checks if the address is valid, then writes the given data into memory. When writing to memory, a signal of -1 is sent to the memory       process via a pipe, handled within the main function.

### Main
The main function ensures proper user usage, error handling, forks the processes, initializes pipes used for IPC, and manages instruction cycles until program execution. A          signal code of -5 signals to the memory process that it is free to close the pipes and exit, ensuring a graceful exit anytime the CPU exits.

## Instruction Set
1 = Load value           
Load the value into the AC   

2 = Load addr            
Load the value at the address into the AC    

3 = LoadInd addr 
Load the value from the address found in the given address into the AC
(for example, if LoadInd 500, and 500 contains 100, then load from 100).

4 = LoadIdxX addr
Load the value at (address+X) into the AC
(for example, if LoadIdxX 500, and X contains 10, then load from 510).

5 = LoadIdxY addr
Load the value at (address+Y) into the AC

6 = LoadSpX
Load from (Sp+X) into the AC (if SP is 990, and X is 1, load from 991).

7 = Store addr
Store the value in the AC into the address

8 = Get
Gets a random int from 1 to 100 into the AC

9 = Put port
If port=1, writes AC as an int to the screen
If port=2, writes AC as a char to the screen

10 = AddX
Add the value in X to the AC

11 = AddY
Add the value in Y to the AC

12 = SubX
Subtract the value in X from the AC

13 = SubY
Subtract the value in Y from the AC

14 = CopyToX
Copy the value in the AC to X

15 = CopyFromX
Copy the value in X to the AC

16 = CopyToY
Copy the value in the AC to Y

17 = CopyFromY
Copy the value in Y to the AC

18 = CopyToSp
Copy the value in AC to the SP

19 = CopyFromSp   
Copy the value in SP to the AC 

20 = Jump addr
Jump to the address

21 = JumpIfEqual addr
Jump to the address only if the value in the AC is zero

22 = JumpIfNotEqual addr
Jump to the address only if the value in the AC is not zero

23 = Call addr
Push return address onto stack, jump to the address

24 = Ret 
Pop return address from the stack, jump to the address

25 = IncX 
Increment the value in X

26 = DecX 
Decrement the value in X

27 = Push
Push AC onto stack

28 = Pop
Pop from stack into AC

29 = Int 
Perform system call

30 = IRet
Return from system call

50 = End
End execution

























