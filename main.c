#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h> //is for usage in header
#include <sys/utsname.h> //is for info in sysinfo
#include <unistd.h> //for sleep/wait function
#include <utmp.h> //is for user info
#include <sys/types.h>
#include <signal.h>
#include "display.h"
#include "usage_functions.h"
#include "pid_obj.h"

#define TRUE 1
#define FALSE 0
#define MAX_STR_LENGTH 1024
#define BUFFER_SIZE 1024


/*
 * @brief handles ctrl + c interrupt by asking user for confirmation
 * @param sig number of thrown signal
 */
void sigint_handler(int __attribute__((unused)) sig) {
    char inpt[16];
    write(STDOUT_FILENO, "\rWould you like to terminate the program? (Y/N): ", 49);
    ssize_t bytes_read = read(STDIN_FILENO, &inpt, sizeof(inpt));
    if (bytes_read > 0)
    {
        if (inpt[0] == 'Y' || inpt[0] == 'y')
        {
            write(STDOUT_FILENO, "Ending program...\n", 18);
            
            // Terminate child processes
            int child_pid;
            for (int i = 0; i < 3; i++) {
                child_pid = get_pid(i);
                if (child_pid > 0) {
                    kill(child_pid, SIGTERM);
                }
            }
            write(STDOUT_FILENO, "\033[0J", 4);
            write(STDOUT_FILENO, "\033[4B", 4);
            exit(0);
        }
        else
        {
            write(STDOUT_FILENO, "\033[A\033[2K", 8); // Move cursor up one line and clear it
            fflush(stdout); // Ensure output is flushed
        }
    }
}

/*
 * @brief sets up pipes and visualizes output based on the returned values
 * @param graphics denotes whether or not graphics should be printed
 * @param sequential denotes whether or not output is sequential
 * @param samples number of samples that should be taken
 * @param tdelay amount of time to be waited inbetween each sample
 * @param users denotes whether or not users should be printed
 */
void printUsage(int graphics, int sequential, int samples, float tdelay, int users) {
    char mem_buffer [BUFFER_SIZE];
    float cpu_buffer;
    char user_buffer [BUFFER_SIZE];

    //CREATE PIPES
    int mem_pipe[2];

    if (pipe(mem_pipe) == -1) { //create memory pipe
        perror("Pipe failed.");
        exit(1);
    }
    int mem_pid = fork(); //fork for memory process
    if (mem_pid < 0) {
        perror("Fork failure.");
        exit(1);
    }
    if (mem_pid == 0) { //enter child process for memory calculations
        close(mem_pipe[0]); //close reading end
        float cur_mem = 0; //use to store previous memory, pass pointer into function
        signal(SIGINT, SIG_IGN); //ignore interrupt signals

        for (int i=0; i<samples; i++) { //loop for all samples
            char* mem_data = getMemDiff(graphics, &cur_mem); //get the memory data
            wait(tdelay);
            size_t mem_data_len = strlen(mem_data); //write it
            if (write(mem_pipe[1], mem_data, BUFFER_SIZE) < (int)mem_data_len) {
                perror("Write to pipe failed."); //error handle write
                exit(1);
            }
        }
        exit(0);
    }
    set_pid(0, mem_pid); //store fork pids so we can terminate

    int cpu_pipe[2];

    if (pipe(cpu_pipe) == -1) {
        perror("Pipe failed.");
        exit(1);
    }
    int cpu_pid = fork(); //fork cpu processes
    if (cpu_pid < 0) {
        perror("Fork failure.");
        exit(1);
    }
    if (cpu_pid == 0) { //cput child process
        close(cpu_pipe[0]); //close reading end
        signal(SIGINT, SIG_IGN); //ignore interrupt signals

        for (int i=0; i < samples; i++) {
            float cpu_usage =  getUsage(tdelay); //wait is baked into getUsage
            ssize_t cpu_data_size = write(cpu_pipe[1], &cpu_usage, sizeof(float)); //write the float
            if (cpu_data_size != sizeof(float)) { //ensure whole float was written
                perror("Write to pipe failed.");
                exit(1);
            }
        }
        exit(0); //close child process
    }
    set_pid(1, cpu_pid); //store fork pids so we can terminate

    int users_pipe[2];

    if (pipe(users_pipe) == -1) {
        perror("Pipe failed.");
        exit(1);
    }
    int users_pid = fork(); //fork cpu processes
    if (users_pid < 0) {
        perror("Fork failure.");
        exit(1);
    }
    if (users_pid == 0) { //users child process
        close(users_pipe[0]); //close reading end
        signal(SIGINT, SIG_IGN); //ignore interrupt signals

        for (int i=0; i < samples; i++) {
            char* user_data = getUsersInfo(); //get the memory data
            wait(tdelay);
            size_t user_data_len = strlen(user_data); //write it
            if (write(users_pipe[1], user_data, BUFFER_SIZE) < (int)user_data_len) {
                perror("Write to pipe failed."); //error handle write
                exit(1);
            }
        }
        exit(0); //close child process
    }
    set_pid(2, users_pid); //store fork pids so we can terminate


    int num_cores = getCores();
    int user_count = 0; //default no users, if we are printing users count them
    if (users) user_count = countConnectedUsers(); //we can assume does not decrease

    //set up reading pipes
    close(mem_pipe[1]);
    close(cpu_pipe[1]);
    close(users_pipe[1]);

    if (!sequential) { //print canvas and set home outside of loop
        printCanvas(samples, graphics, num_cores, user_count);
        printf("\033[%dA", (graphics*samples + 2)); //go to cpu usage line
    }
    //the BIG printing loop
    for(int i=0; i<samples; i++) {
        if (sequential) { //on every iteration print header and canvas
            printf("Iteration: %d\n", i+1); //track iteration #s, wont effect escape code travelling cause dont ever go this high
            printHeader(samples, tdelay);  //every iteration, new header and new canvas
            printCanvas(samples, graphics, num_cores, user_count);
            printf("\033[%dA\r", (graphics*samples + 2)); //go to cpu usage line
        }
        
        if (read(cpu_pipe[0], &cpu_buffer, sizeof(float)) > 0) {//read cpu usage from pipe
            printf("total cpu use = %.2f%%", cpu_buffer);
        }
        else { //error reading
            perror("Reading from pipe error.");
            exit(1);
        }
        if (users) {
            printf("\033[%dA\r", user_count + 3); //up to user start

            if (read(users_pipe[0], user_buffer, BUFFER_SIZE)> 0) {
                printf("%s\n", user_buffer);
            }
            else {
                perror("Reading from pipe error.");
                exit(1);
            }

            printf("\033[%dA\r", user_count + samples - i + 2); //up to current memory spot
        }
        else {
            printf("\033[%dA\r", 2 + samples - i);
        }

        if (read(mem_pipe[0], mem_buffer, BUFFER_SIZE) > 0) {
            printf("%s\n", mem_buffer);
        }
        else {
            perror("Reading from pipe error.");
            exit(1);
        }
        if (users) {//back down to cpu usage
            printf("\033[%dB\r", samples - i + user_count + 3); //with users
        }
        else {
            printf("\033[%dB\r", samples - i + 1); //without users
        }
        

        if (graphics) { //print the cpu graphics
            printf("\033[%dB\r", i+1);
            cpuGraphic(cpu_buffer);
            printf("\033[%dA\r", i + 2);
        }

        if (sequential) {
            printf("\033[%dB\r", graphics*samples + 2);
            printSysInfo();
        }
    }
    printf("\033[%dB\r", graphics*samples + 2); //back down to bototm
}

/*
 * @brief visualizes the case where only user data is requested
 * @param samples number of samples to be taken
 * @param tdelay amount of time between each sample
 * @param sequential denotes whether output should be sequential
 */
void printUsers(int samples, float tdelay, int sequential) { //only 1 process, no need for pipes.
    int user_count = 0;
    for(int i=0; i<samples; i++) {
        if (sequential) {
            printf("iteration: %d\n", i+1);
            printHeader(samples, tdelay);
        }
        user_count = countConnectedUsers();
        printf("%s", getUsersInfo());
        wait(tdelay);
        if(sequential) printSysInfo();
        else {    
            if (i!= samples-1) {     
                for(int j=0; j<user_count+1; j++) {
                    printf("\033[A");
                    printf("\033[K");
                }   
            }
        }        
    }
}

/*
 * @brief main function, sets signal interrupts and parses cmd line arguments
 */
int main(int argc, char **argv) {  
    signal(SIGTSTP, SIG_IGN); //intercept ctrl+z and ignore (no reason to run in background)
    signal(SIGINT, sigint_handler); //intercept ctrl+c

    int samples = 10;
    float frequency = 1;
    int usageType= 0;
    int graphics = FALSE;
    int sequential = FALSE;
    if (argc>1) {
        for(int i=1; i<argc; i++) { //check given flags to determine what we will need to output
            if (!(strcmp(argv[i], "--system"))) {
                if (usageType == 2) usageType = 0;
                else usageType = 1;
            }
            else if (!(strcmp(argv[i], "--user"))) {
                if (usageType == 1) usageType = 0;
                else usageType = 2;
            }
            else if (!(strcmp(argv[i], "--graphics")) || !(strcmp(argv[i], "-g"))) {
                graphics = TRUE;
            }
            else if (!(strcmp(argv[i], "--sequential"))) {
                sequential = TRUE;
            }
            else if (!(strncmp(argv[i], "--samples=", 10))) {
                samples = atoi(&argv[i][10]);
            }
            else if (!(strncmp(argv[i], "--tdelay=", 9))) {
                frequency = atof(&argv[i][9]);
            }
        }
        if ((strtol(argv[argc-2], NULL, 10)) || !(strncmp(argv[argc-2], "--samples=", 10))) { //this is for positional arguments 
            if (strtof(argv[argc-1], NULL) || !(strncmp(argv[argc-1], "--tdelay=", 9))) {   //samples and tdelay can be passed in the last spot as numbers and this will
                float temp = strtol(argv[argc-2], NULL, 10);                                //parse that and pass the correct arguments
                if (temp) samples = (int)temp;
                else samples = atoi(&argv[argc-2][10]); 
                
                temp = strtof(argv[argc-1], NULL);
                if (temp) frequency = temp;
                else frequency = atof(&argv[argc-1][9]);
            }
        }
    }

    if(!sequential) printHeader(samples, frequency); //not sequential means 1 header
    if(usageType == 2) {
        printUsers(samples, frequency, sequential); //only users
    }
    else if (usageType == 1) {
        printUsage(graphics, sequential, samples, frequency, 0); //no users
    }
    else{
        printUsage(graphics, sequential, samples, frequency, 1); //everything
    }
    if(!sequential) printSysInfo(); //not sequential means 1 footer

    return 0;
}
