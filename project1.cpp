/*
    Program: Computer Simulator
    File:    Project1.cpp
    Author:  Stanton Brown
    Date:    February 24, 2024

    Desription:
    C++ program simulating a computer system with a CPU and memory using two processes and pipes.
    The program reads a file containing a program to be executed by the CPU.
    CPU and memory processes communicate through pipes, executing instructions and interacting based on the program.
    
    - Memory Class: Represents computer memory, initializes from file, and provides read/write functionalities.
    - CPU Class: Represents the Central Processing Unit, executes instructions, handles interrupts, and manages registers.
    - Main Function: Sets up pipes, forks processes, and manages continuous execution of CPU and memory until exit.
    
    Instruction set includes arithmetic, memory manipulation, system calls, execution manipulation, and proper error handling.
    Supports timer interrupts and system call interrupts, entering kernel mode, and saving context on the system stack.
    Child and parent processes communicate via pipes, and the program exits gracefully with resource release.
    
    Proper memory permissions are ensured

    Usage:
    To run the program, use the following command in the terminal:
    ./program_name input_file timer_value

    - program_name: The name of the compiled program.
    - input_file:   The name of the file containing the program to be executed.
    - timer_value:  An integer specifying the timer constraint for interrupt handling.
*/


#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <stdexcept>

using namespace std;


/*
 * Memory: Represents the computer's memory
 * ----------------------------------------
 * This class represents the memory of the computer.
 * It includes functionalities to read and write data into memory.
 * The memory size is fixed at 2000 and is initialized from an input file.
 */
class Memory{

private:
    //Specifies the size of the memory
    static const int MEMORY_SIZE = 2000;

    //Used to store user program and system memory
    int memory[MEMORY_SIZE];

public:

    /*
     * Constructor: Memory
     * ----------------
     * Initializes the Memory by calling on the readInputFile function
     * Parameters:
     * - inputFile: the file that represents the program to be executed
     */
    Memory(const char* inputFile){
        readInputFile(inputFile);

    }

    /*
     * Function: readInputFile 
     * -------------------
     * Initializes the memory with data from the input file.
     * Reads the input file and populates the memory accordingly.
     * Parameters:
     * - fileName: the name of the input file containing initial memory data.
     */
    void readInputFile(const char* fileName){
        ifstream inputFile(fileName);
        if(!inputFile.is_open()){
            cerr << "ERROR: unable to open the input file" << endl;
            exit(1);
        }

        string line;
        int memoryIndex = 0;

        //Loop through the entire file line by line
        while (getline(inputFile, line)) {
            istringstream iss(line);
            //Check if we are loading to a new address
            if (iss.peek() == '.'){
                int newIndex;
                iss.ignore();

                //read in the new index ignoring space
                while(iss >> newIndex){
                    if (iss.peek() == ' ') {
                        iss.ignore();
                    }
                    else {
                        break; 
                    }
                }

                //set new memory index and continue
                memoryIndex = newIndex;
                continue;
            }

            //Read instruction into memory while ignoring space 
            while (iss >> memory[memoryIndex]) { 
                ++memoryIndex;
                if (iss.peek() == ' ') {
                    iss.ignore(); 
                }
                else {
                    break;
                }
            }
        }

        inputFile.close(); 

        // // Memory check
        // for(int i = 0; i <= 260; i++){
        //     //cout << "Value at address "<< i << ": " << memory[i] << endl;
        // }

    }

    /*
     * Function: read 
     * -------------
     * Reads data from memory at the specified address.
     * Parameters:
     * - address: the memory address to read from.
     * Returns:
     * The value stored at the specified memory address.
     */
    int read(int address) const {
        if (address < 0 || address >= MEMORY_SIZE) {
            cerr << "ERROR: Invalid memory address accessed: " << address << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }

        return memory[address];
    }

    /*
     * Function: write 
     * --------------
     * Writes data to memory at the specified address.
     * Parameters:
     * - address: the memory address to write to.
     * - data: the data to be written to memory.
     */
    void write(int address, int data) {
        if (address < 0 || address >= MEMORY_SIZE) {
            cerr << "ERROR: Invalid memory address accessed: " << address << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }

        memory[address] = data;
    }

};


/*
 * CPU: Represents the Central Processing Unit
 * -------------------------------------------
 * This class represents the Central Processing Unit (CPU) of the computer.
 * It executes instructions and interacts with memory through communication pipes.
 * The CPU includes various registers and supports interrupt handling.
 */
class CPU {
public:
    //Registers
    int PC; //Program Counter
    int SP; //Stack Pointer
    int IR; //Instruction Register
    int AC; //Accumulator
    int X;  //Additional
    int Y;  //Additional

    //Innerprocess Communicaion
    int pfds_mem;   //used to read from memory
    int pfds_cpu;   //used to write to memory (request next instruction)

    //mode
    bool kernelMode;

    //Timer for interrupts
    int timer;
    const int timeConstraint;

    //enable/disable interupts 
    bool interuptEnabled;

    //local variables
    int operand;    //used for program operations
    int signal;     //used to signal to memory (write, exit)

    /*
     * Constructor: CPU 
     * ----------------
     * Initializes the CPU with communication pipes and timer constraints.
     * Parameters:
     * - pfds_1: file descriptor for communication with memory (writing)
     * - pfds_2: file descriptor for communication with memory (reading)
     * - tCon: time constraint for interrupt handling
     */
    CPU(int pfds_1, int pfds_2, int tCon) : pfds_cpu(pfds_1), pfds_mem(pfds_2), timeConstraint(tCon),
    interuptEnabled(true), kernelMode(false), PC(0), SP(1000), AC(0), X(0), Y(0), timer(0) {}

    /*
     * Function: executeInstruction
     * ----------------------------
     * Executes the instruction currently in the Instruction Register (IR).
     * Handles various instructions based on the opcode in IR.
     * Manages system calls and execution of instructions.
     */
    void executeInstruction() {

        //Check if a timer interupt has occured
        timerInterupt();

        //Execute the instruction in the Instruction Register
        switch (IR) {
            case 1:
                //Load value
                // Load the value into the AC     

                //Fetch the value
                fetchOperand();
            
                //Load value into AC
                //cout<< "Value loaded into AC: "<< operand << endl;
                AC = operand;
                break;

            case 2:
                // Load addr
                // Load value at the address into AC

                //Fetch the address
                fetchOperand();
                //cout<< "Address "<< operand << endl;

                //Fetch the value at address
                readMemory(operand);
                //cout<< "Value at address "<< operand << endl;

                //Assign Accumulator to that value
                AC = operand;
                break;

            case 3:
                //LoadInd addr   
                // Load value from the address found in the given address into AC
                //address -> address -> value 

                //Feth intial address value
                fetchOperand();
                //cout << "Intial address: " << operand << endl;

                //Fetch new address at intial address
                readMemory(operand);
                //cout<< "New address: "<< operand << endl;

                //Fetch value at new address
                readMemory(operand);
                //cout<< "Value at new address: "<< operand << endl;

                AC = operand;
                break;

            case 4:
                //LoadIdxX addr
                // Load value at (address+X) into AC ?

                //fetch the address 
                fetchOperand();

                //Add X to that adress
                //cout << operand  << "+" << X << "=";
                operand += X;
                //cout << operand << endl;

                //Read the memory value at (address + X)
                readMemory(operand);
                //cout<< "Value at address: "<< operand << endl;

                //Set Accumulator  
                AC = operand;
                break;

            case 5:
                // Load value at (address+Y) into AC

                //fetch the address 
                fetchOperand();

                //Add Y to that adress
                operand += Y;

                //Read the memory value at (address + Y)
                readMemory(operand);

                //Set Accumulator to 
                AC = operand;
                
                break;

            case 6:
                // Load from (SP+X) into AC

                //Add X to that adress
                operand = SP + X;

                //Read the memory value at (SP + X)
                readMemory(operand);

                //Set Accumulator to 
                AC = operand;
                
                break;

            case 7:
                // Store the value in AC into the address

                //Fetch address
                fetchOperand();

                //Wite Accumaulator into address
                writeMemory(operand, AC);
                break;

            case 8:
                // Get a random int from 1 to 100 into the AC
                AC = rand() % 100 + 1;
                //cout<< "Random number:" << AC;
                break;

            case 9:
                //If operand=1, writes AC as an int to the screen
                //If operand=2, writes AC as a char to the screen

                //Fetch the operand for this instruction
                fetchOperand();
                
                if(operand == 1){
                    cout << AC;
                }
                else if(operand == 2) {
                    cout << char(AC);
                }
                else{
                    cerr << "Invalid operand for instruction 9.." << endl;
                }
                
                break;

            case 10:
                // Add the value in X to the AC
                //cout << "AC = " << AC << "+" << X << endl;
                AC += X;
                //cout << "AC = " << AC << endl;
                break;

            case 11:
                // Add the value in Y to the AC
                //cout << "AC = " << AC << "+" << Y << endl;
                AC += Y;
                //cout<< "AC = " << AC << endl;
                break;

            case 12:
                //Subtract the value in X from the AC

                AC -= X;
                break;

            case 13:
                //Subtract the value in Y from the AC
                AC -= Y;
                break;

            case 14:
                // Copy the value in the AC to X
                X = AC;
                //cout<< "X = " << X << endl;
                break;

            case 15:
                //Copy the value in X to the AC
                AC = X;
                break;

            case 16:
                //Copy the value in the AC to Y
                Y = AC;
                //cout<< "Y = " << Y << endl;
                break;

            case 17:
                // Copy the value in Y to the AC
                AC = Y;
                //cout<< "AC = " << AC << endl;
                break;

            case 18:
                //Copy the value in AC to the SP
                SP = AC;
                break;

            case 19:
                //Copy the value in SP to the AC 
                AC = SP;
                break;

            case 20:
                //Jump to the address

                //Fetch address to jump to 
                fetchOperand();
                PC = operand;

                //Jump to that address
                //cout << "Jumping to address: " << PC << endl;
                fetchInstruction();
                executeInstruction();
                break;

            case 21:
                //JumpIfEqual addr
                //Jump to the address only if the value in the AC is zero

                //cout << "AC = " << AC << endl;

                if(AC == 0){

                    //Fetch address to jump to 
                    fetchOperand();
                    PC = operand;

                    //Jump to address
                    //cout << "Jumping to address: " << PC << endl;
                    fetchInstruction();
                    executeInstruction();
                }
                else{
                    //Need to incrment PC to skip operand 
                    //cout << "Continue normal execution: " << endl;
                    PC++;
                }

                break;
            case 22:
                //JumpIfNotEqual addr
                //Jump to the address only if the value in the AC is not zero
                
                //cout << "AC = " << AC << endl;

                if(AC != 0){
                    //Fetch address to jump to 
                    fetchOperand();
                    PC = operand;

                    //Jump to that address
                    //cout << "Jumping to address: " << PC << endl;
                    fetchInstruction();
                    executeInstruction();
                }
                else{
                    //Need to incrment PC to skip operand 
                    //cout << "Continue normal execution: " << endl;
                    PC++;
                }

                break;

            case 23:
                //Call addr
                //Push return address onto stack, jump to the address

                fetchOperand();

                //Push the return address onto stack
                pushStack(PC);

                PC = operand;

                //Perform procedure call
                //cout << "Jumping to address: " << PC << endl;
                //cout << "Timer: " << timer << endl;
                fetchInstruction();
                executeInstruction();
                break;

            case 24:
                //Pop return address from the stack, jump to the address
                PC = popStack();
                //cout << "End procedure call ~~~~~~~~~~~~~~ " << endl;
                break;

            case 25:
                // Increment the value in X
                X++;
                //cout << "X = " << X;
                break;

            case 26:
                // Decrement the value in X
                X--;
                //cout << "X = " << X;
                break;

            case 27:
                //push
                //Push AC onto stack
                pushStack(AC);
                //cout << "Pushed: " << AC << endl;
                break;
            case 28:
                //pop
                // Pop from stack into AC
                AC = popStack();
                break;

            case 29:
                //int 
                // Perform system call
                //cout << "System call occured" << endl;
                //cout << endl << "Registers prior to interupt" << endl;
                // printRegisters();
                
                //Enter kernel mode 
                kernelMode = true;

                //"The SP and PC registers (and only these registers) should be saved on the system stack BY THE CPU."
                //Temporarily store user SP in operand 
                operand = SP;

                //Set SP to system stack
                SP = 2000; 

                //Push the user SP onto the system stack
                pushStack(operand);

                //Push the user Program Counter onto the system stack
                pushStack(PC);

                // The int instruction should cause execution at address 1500
                // Set to 1499 because the driver in main() will increment after execution is complete
                interruptHandler(1);

                break;
                

            case 30:
                //IRet
                // Return from system call

                

                //restore user program context
                Y = popStack();
                X = popStack();
                AC = popStack();
                IR = popStack();
                PC = popStack();
                SP = popStack();

                //cout << "Registers after interupt: " << endl;
                // printRegisters();

                //Set mode back to user mode
                kernelMode = false;

                //enable interupts
                interuptEnabled = true;

                break;

            case 50:
                // End execution
                // Writes signal -5 to memory to indicate exit
                // //cout << "CPU EXITING..." << endl;
                signal = -5;
                write(pfds_cpu, &signal, sizeof(signal));

                //close pipes
                close(pfds_cpu);
                close(pfds_mem);
                exit(0);
                
                break;

            default:
                // Handle invalid instruction
                cerr << "ERROR: Invalid instruction: " << IR << endl;
                exit(1);
                break;
        }
    }

    
    /*
     * Function: timerInterupt
     * -----------------------
     * Checks if the instruction count has exceeded the timer constraint. If so,
     * triggers a timer interrupt by entering kernel mode and saving the user program context
     * on the system stack. Then calls the interrupt handler for timer interrupts.
     * If the timer condition is not met, increments the timer.
    */
    void timerInterupt(){
        //Check if the instruction count has exceeded the timer
        if(interuptEnabled && timer >= timeConstraint){
            //cout << "Timer Interupt occured" << endl;
            //cout << endl << "Registers prior to interupt" << endl;
            // printRegisters();
    
            //Enter kernel mode 
            kernelMode = true;

            //"The SP and PC registers (and only these registers) should be saved on the system stack BY THE CPU."
            //Temporarily store user SP in operand 
            operand = SP;

            //Set SP to system stack
            SP = 2000; 

            //Push the user SP onto the system stack
            pushStack(operand);

            //Push the user Program Counter onto the system stack
            pushStack(PC);

            //Call interupt handler
            interruptHandler(0);

            //cout << "Interupt handling complete continuing execution" << endl;
            //cout << "PC:" << PC << endl;
            //cout << "IR:" << IR << endl;

        }
        else{
            //increment the timer
            timer++;
        }
    }
    

    /*
     * Function: popStack
     * -------------------
     * Pops a value from the stack.
     * Reads the value from memory at the current stack pointer (SP) and increments SP.
     * Returns:
     * The popped value.
     */
    int popStack(){
        //cout << "Pop stack at index: " << SP << endl;

        //Ensure proper permission
        checkPermission(SP);

        //used to read in what was on the stack 
        int data;

        //Requesting value at top of stack
        write(pfds_cpu, &SP, sizeof(SP));

        //Reading that value
        read(pfds_mem, &data, sizeof(data));

        //increment stack
        SP++;

        //cout << "Pop value: " << data << endl;
        
        return data;
    }

    /*
     * Function: pushStack
     * --------------------
     * Pushes a value onto the stack.
     * Decrements SP, then writes the address and data to memory at the new SP.
     * Parameters:
     * - data: The value to push onto the stack.
     */
    void pushStack(int data){

        //decrement stack pointer
        SP--;

        //Ensure proper permission
        checkPermission(SP);

        //cout << "Push stack at index = " << SP << endl;

        //signal to the memory cpu is about to write 
        signal = -1;
        write(pfds_cpu, &signal, sizeof(signal));

        //Write data to memory at given address
        write(pfds_cpu, &SP, sizeof(SP));
        write(pfds_cpu, &data, sizeof(data));
    }

    /*
     * Function: writeMemory
     * ---------------------
     * Writes data to the specified memory address.
     * Notifies memory before writing and sends the address along with the data.
     * Parameters:
     * - address: The memory address to write to.
     * - data: The data to write to the memory address.
     */
    void writeMemory(int address, int data){

        //Ensure proper permission
        checkPermission(address);

        //Notify memory we are preparing to write 
        signal = -1;
        write(pfds_cpu, &signal, sizeof(signal));

        //Write data to memory at given address
        write(pfds_cpu, &address, sizeof(address));
        write(pfds_cpu, &data, sizeof(data));
    }

    /*
     * Function: readMemory
     * --------------------
     * Reads data from the specified memory address.
     * Ensures permissions are correct prior to reading.
     * Parameters:
     * - address: The memory address to read from.
     */
    void readMemory(int address){

        //Ensure proper permission
        checkPermission(address);
        
        //Fetch memory at address
        // //cout <<"Reading from address: " <<address<<endl;
        write(pfds_cpu, &address, sizeof(address));
    
        //Read the returned memory
        read(pfds_mem, &operand, sizeof(operand));
        // //cout <<"Read: " << operand<<endl;
    }

    
    /*
     * Function: fetchInstruction
     * -------------------------
     * Fetches the next program instruction from memory.
     * Sends the current Program Counter (PC) to memory and reads the instruction from memory.
     */
    void fetchInstruction(){

        //Fetch next program instruction
        write(pfds_cpu, &PC, sizeof(PC));
        //cout << endl << "CPU fetch at index: " << PC << endl;

        //Read the program instruction from memory
        read(pfds_mem, &IR, sizeof(IR));
        //cout << "CPU executing instruction: " << IR << endl;
    }

    /*
     * Function: fetchOperand
     * ----------------------
     * Fetches the operand for the current instruction.
     * Increments PC, sends the updated PC to memory, and reads the operand from memory.
     */
    void fetchOperand(){
            PC++;
            write(pfds_cpu, &PC, sizeof(PC));
            read(pfds_mem, &operand, sizeof(operand));
            // //cout << "CPU READ OPERAND: " << operand << endl;
            
    }

    /*
     * Function: interruptHandler
     * --------------------------
     * Handles interrupts (timer or system calls).
     * Saves the current CPU context on the system stack, enters kernel mode,
     * and calls the appropriate interrupt handler (timer or system call).
     * Parameters:
     * - code: The code indicating the type of interrupt (0 for timer, 1 for sys call).
     */
    void interruptHandler(int code){

        //Ensure proper permission
        checkPermission(PC);

        //avoid nested exectution of interupts 
        interuptEnabled = false;

        //save context for user program to system stack 
        pushStack(IR);
        pushStack(AC);
        pushStack(X);
        pushStack(Y);

        //Check what interupt we are handling
        if(code == 0){
            //timer interupt
            //reset timer 
            timer = 0;

            //Set Program Counter to 1000 
            PC = 1000; 

            //Loop instruction cycles until kernel mode is disabled 
            while(kernelMode){
                //Fetch next program instruction
                fetchInstruction();

                //Execute program instruction
                executeInstruction();
                //cout << "Timer: " << timer << endl;

                //Increment Program Counter if we are still in kernel mode
                //Instruction 30 would have changed still
                //Can not use instruction 30 as condition because it has already restored user state
                if(kernelMode){
                    PC++;
                }

            }
        }
        else if(code == 1){
            //sys call

            //Set Program Counter to 1500
            // ?? may need to be 1499 because PC will increment 
            PC = 1500; 

            //Loop instruction cycles until kernel mode is disabled 
            while(kernelMode){
                //Fetch next program instruction
                fetchInstruction();

                //Execute program instruction
                executeInstruction();
                //cout << "Timer: " << timer << endl;

                //Increment Program Counter if we are still in kernel mode
                if(kernelMode){
                    PC++;
                }

            }
        }
        else{
            cerr << "ERROR: Invalid interupt signal" << endl;
            //cout << "Exiting..." << endl;
            _exit(1);
        }
   
    }

    /*
     * Function: checkPermission
     * --------------------------
     * Check if the user program is attempting to access system memory
     * Parameters:
     * - address: the address the program is attempting to access
     */
    void checkPermission(int address){
        if(address >= 1000 && !kernelMode){
            cerr << "ERROR: User can not access system memory" << endl;
            cerr << "Exiting..." << endl;
            _exit(1);
        }
    }

    /*
     * Function: printRegisters
     * ------------------------
     * Prints the values of all registers for debugging purposes.
     */
    void printRegisters(){
        //cout << "PC: " << PC << endl;
        //cout << "SP: " << SP << endl;
        //cout << "IR: " << IR << endl;
        //cout << "AC: " << AC << endl;
        //cout << "X: " << X << endl;
        //cout << "Y: " << Y << endl << endl;
    }
};



/*
 * Main Function
 * -------------
 * The main function that sets up communication pipes, forks the process,
 * and manages the cpu and memory process.
 * Parameters:
 * - argc: the number of command-line arguments.
 * - argv: an array of command-line arguments.
*/
int main(int argc, char *argv[]) {

    // Used to store the command line argument 
    int timerInput;

    //used for fork
    pid_t pid;

    //Check for proper usage 
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <file name> <timer>" << endl;
        _exit(1);
    }

    //Ensure argumetn is an integer
    try {
        stringstream container(argv[2]);
        int x;
        container >> timerInput;
        // cout << "Value of x: " << timerInput;
        } 
        catch (const invalid_argument& e) {

            // Failed to convert
            cerr << "ERROR: Second argument must be an integer" << endl;
            cerr << "Exiting..." << endl;
            _exit(1);
        }


    //pipe to write from cpu to memory
    int pfds_cpu[2];    //CPU -> Mem

    //pipe to write from memory to cpu
    int pfds_mem[2];    //Mem -> CPU

    //Seed to ensure we arent producing the same random number with instruction
    //Must be called here because if called within the instruction it produces the same integer
    srand(time(NULL));

    //Check if pipes failed
    if(pipe(pfds_cpu) == -1){
        cerr << "ERROR: The cpu pipe failed" << endl;
        exit(1);
    }
    if(pipe(pfds_mem) == -1){
        cerr << "ERROR: The memory pipe failed" << endl;

        //close the open pipes
        close(pfds_cpu[0]);
        close(pfds_cpu[1]);
        exit(1);
    }

    //spawn the child process (CPU)
    pid = fork();

    //Check if Fork failed
    if(pid == -1){
       cerr << "ERROR: The fork failed" << endl;
       exit(1);
    }
    else if(pid == 0){
        //Child process (CPU)
        CPU cpu(pfds_cpu[1], pfds_mem[0], timerInput);

        //Close unused pipe ends
        close(pfds_cpu[0]);
        close(pfds_mem[1]);

        //Instruction cycle loop until program ends
        while(true){
            
            //Fetch next program instruction
            cpu.fetchInstruction();

            //Execute program instruction
            cpu.executeInstruction();
            //cout << "Timer: " << cpu.timer << endl;

            //Increment Program Counter
            cpu.PC++;

        }

    }
    else{
        //Parent process (Memory)

        //Do not need to read from this end of this pipe
        close(pfds_mem[0]);
        close(pfds_cpu[1]);

        //Used to store when user is writing data to some address
        int address;
        int data;
        
        //Used to read and write to CPU
        int instruction;
        int buf;


        //Initiate memory with the input program
        Memory memory(argv[1]);

        //Enter loop until cpu exits
        while(true){

            //Recieve request from cpu
            read(pfds_cpu[0], &buf, sizeof(buf));
            // //cout << "MEMORY Read: " << buf << endl;

            //CPU attempting to write to memory
            if(buf == -1){

                //Read the address cpu wants to write to
                read(pfds_cpu[0], &address, sizeof(address));
                // //cout << "MEMORY Read adress: " << address << endl;

                //Read the data cpu wants to write
                read(pfds_cpu[0], &data, sizeof(data));
                // //cout << "MEMORY Read Data: " << data << endl;

                //write the data to the address
                memory.write(address, data);

            }
            else if(buf == -5){
                //CPU Exiting 
                waitpid(-1, NULL, 0);
                // //cout << "MEMORY Exiting..." << endl;
                close(pfds_mem[1]);
                close(pfds_cpu[0]);
                exit(0);
            }
            else{
                //Read from memory
                instruction = memory.read(buf);
                // //cout << "Instruction in memory: " << instruction << endl;

                //Return program instruction to cpu
                write(pfds_mem[1], &instruction, sizeof(instruction));
                // //cout << "MEMORY WRITE: " << instruction << endl;
            }
        }   
    }
}