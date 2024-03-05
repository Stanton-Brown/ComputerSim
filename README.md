Project Purpose:
The purpose of this project was to simulate a simple computer system, comprising of a Central Processing Unit (CPU) and Memory, emphasizing the comprehension of inter-process communication (IPC) and numerous low-level concepts that are pivotal to an operating system working properly. Some of these low-level concepts include processor interaction with main memory, processor instruction behavior, the role of registers, stack processing, procedure and system calls, handling of interrupts, memory protection, and I/O. It is important to note that this project was done for CS 4348 Operating Systems at the University of Texas at Dallas. 

Usage:
To run the program, use the following command in the terminal:
    ./program_name input_file timer_value

    - program_name: The name of the compiled program.
    - input_file:   The name of the file containing the program to be executed.
    - timer_value:  An integer specifying the timer constraint for interrupt handling.


Implementation:
I chose to implement my project in C++ as I am more familiar with the handling of IPC within this language. In order to enhance the readability of my project I took an object-oriented approach to designing the CPU and Memory modules while using the main function as a driver for instruction cycles and communication between the two processes. A description of each of these classes is provided below:

CPU:
The CPU class contains all necessary functions for instruction execution. It additionally contains functions for interrupt handling and memory protection, which I got approval to include within this class from Professor Ozbirn. 

Instruction Execution:
Within the CPU class there is an executeInstruction() function which will be called at least once within every instruction cycle. Prior to the execution of the instruction contained within the Instruction Register (IR), the function will first ensure that a timer interrupt has not occurred. It then proceeds to read from the IR and execute the instruction accordingly. 

Memory Access:
Anytime the CPU is attempting to read or write to/from memory the CPU ensures that system code is not being accessed without the proper permissions. As there are numerous instructions that access memory I have implemented a function checkPermissions(int address) that will check if the user program is attempting to access system memory. If this is the case and kernel mode is not enabled the program will indicate an error and exit gracefully. Additionally, anytime the CPU is attempting to write to memory it will signal the memory module by writing -1 to the pipe. This allows the memory module to prepare itself to read in an address followed by the data to be written. 

Interrupt Handler:
In order to handle both timer interrupts and system calls the interruptHandler(int code) was implemented. It takes as input an integer that represents what kind of interrupt we are currently handling. An integer value of 0 indicates a timer interrupt and a value of 1 indicates a system call. Upon being called the function will ensure proper permissions, disable other interrupts to avoid nested execution, save the context of the user program on the system stack, jump to the appropriate address based on the signal received, and enter a loop that executes instruction cycles until the interrupt has been fully handled. 

Memory:
The Memory class is fairly simple, it contains functions to initialize memory, read from memory, and write to memory. 

Memory Initialization:
Upon being created the memory will be initialize by calling the function readInputFile(const char* fileName) which takes as input the name of the user program to be ran. It ensures that the file is read into an integer array properly by skipping over any invalid characters and changing the index any time a ‘.’ is encountered. 

Read Memory:
When reading from memory the read(int address) function is called. It simply checks that a valid address is being read and returns the value at that address. 

Write Memory:
Similar to the read function, when writing to memory the write(int address, int data) function simply checks if the address is valid, then writes the given data into memory. As mentioned prior anytime the CPU is attempting to write to memory a signal of -1 is sent to the memory process via a pipe which is handled within the main function. 
 
Main:
The main function within the program is used to ensure proper user usage, error handling, fork the processes, initialize the pipes used for IPC, and manage instruction cycles until program execution. To ensure a graceful exit anytime the CPU is exiting it signals to the memory process via a pipe with the signal code -5. This signal code tells the memory process it is free to close the pipes and exit. 
