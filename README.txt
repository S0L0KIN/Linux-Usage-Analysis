		-- HOW DID I SOLVE THE PROBLEM --
Set up three different pipes and child processes for each required process:
	1. Memory usage
	2. CPU usage
	3. Connected users
Each child process writes to their pipe samples times on length tdelay intervals, the parent process loops reading each one (also samples times) in sequence and printing it to the correct given location.

Another issue was to handle the child processes when CTRL+C was called. To resolve this, when each child process is created their pid is stored statically using pid_obj. When interrupt is called, those pids are killed before exiting the program.


		-- OVERVIEW OF FUNCTIONS --
	--- in display
/*
 * @brief prints row of -'s as a divider
 */
void printDivider();

/*
 * @brief prints the header portion of the output
 * @param samples amount of samples to be taken
 * @param frequency amount of time between each sample
 */
void printHeader(int samples, float frequency);

/*
 * @brief prints the system info at the footer of the visualization
 */
void printSysInfo();

/*
 * @brief prints the static outline leaving space for the dynamic data to be filled in
 * @param samples amount of samples to be taken
 * @param graphics denotes whether or not graphics should be printed
 * @param num_cores number of cores the CPU has 
 * @param users denotes whether or not users should be printed
 */
void printCanvas(int samples, int graphics, int num_cores, int users);

/*
 * @brief visualizes the amount of cpu usage with |'s
 * @param delta current usage
 */
void cpuGraphic(float delta);

	--- in usage_functions
/*
 * @brief sleeps the system for a float amount of time
 * @param tdelay amount of time to sleep for
 */
void wait(float tdelay);

/*
 * @brief counts how many users are connected at time of calling
 * @return the number of connected users
 */
int countConnectedUsers();

/*
 * @brief gets connected user information
 * @return string of connected user information
 */
char* getUsersInfo();

/*
 * @brief computes memory usage at time of calling
 * @param graphics denotes whether graphical visualization of change should be appended
 * @param prev_memory pointer to float containing previous memory usage
 * @return string with memory information + optional graphical visualization (if graphics)
 */
char *getMemDiff(int graphics, float *prev_memory);

/*
 * @brief counts the number of cpu cores
 * @return the number of cores
 */
int getCores();

/* 
 * @brief calculates cpu usage over time interval
 * @param tdelay length of that interval
 * @return the cpu usage
 */
float getUsage(float tdelay);

	--- in pid_obj
/*
 * @brief sets process id into array
 * @param index is the index of insertion
 * @param pid is the childs process id
 */
void set_pid(int index, int pid);

/*
 * @brief gets process id at given spot
 * @param index is the desired pid  in array
 * @return the process id  
 */
int get_pid(int index);


		-- HOW TO USE THE PROGRAM --
make [command]
	all	: creates SystemUsage  
	clean	: removes SystemUsage and .o files
	help	: prints this menu to console
		
SystemUsage accepts the following arguments:
--system     	: solely displays system data (no user data)
--user        	: solely displays user data (no system data)
--graphics     	: provides visualization for CPU and memory data
-g        	: equivalent to --graphics
--sequential    : changes the output to be sequential (i.e. instead of live updates, prints one after another)
--samples=#    	: replace # with an integer to change the number of data points taken
--tdelay=#    	: replace # with a float to change the amount of time (in seconds) between each sampling

!!! IMPORTANT POINTS OF USE !!!
The final 2 arguments may also be passed as positional arguments in the order 'int float', 
this will be read as '--samples=int --tdelay=float'.
Both must be present and in this order for the positional arguments to be accepted.

If duplicate arguments are passed, the last instance of the argument will be considered (i.e. two calls to --samples=# will override
eachother), if neither or both --system and --user are passed then both sets of information will be output. It is never needed to
pass the same argument multiple times.

If unreasonable (considering typical) amounts of users are connected, CPU is being used, or memory is being used occassionally 
the terminal will not provide enough space for output and graphics may overlap/spill into other lines.
In this case, pass the 'clear' argument to your terminal after the program terminates and attempt to run the program again.
(This might also occur if the terminal is atypically small, or a number of other abnormal conditions)

Visualization of users handles any number of users but expects the user count to not change after printing begins or formatting will be affected.

calling 'make' with no additional command is equivalent to calling 'make all'

