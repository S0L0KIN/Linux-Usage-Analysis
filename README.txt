	an overview of the functions (including documentation)
void printDivider() 
// prints out a line of hyphens, used to seperate the different blocks of data
// put in one function as it makes the code more readable (and saves the programmer hassle of counting hyphens every time)

void printHeader(int samples, float frequency) 
// takes samples representing how many data points we will print and frequency representing how long between each data point
// prints those variables as well as uses <sys/resource.h> for struct rusage to print self diagnostic data

void printSysInfo()
// prints system data using <sys/utsname.h> for struct utsname
// parses and reformats /proc/uptime to print machine uptime

void wait(float tdelay) 
// uses <unistd.h> to combine sleep and usleep to create one function that can wait for floats >1 amount of time

int printUsersInfo() 
// prints user data on the users currently connected
// returns the number of users/sessions currently connected

float printMemDiff(int graphics, float prev_memory, float total_phymem, float total_virtmem)
// takes total_phymem and total_virtmem as arguments only so it doesnt repeatedly strtof the values from the file (avoids complexity)
// the above arguments represent the total physical memory and total virtual memory of the machine
// parses file /proc/meminfo to calculate how much memory is being used total (both phys and virt)
// uses prev_memory to display the difference in memory usage between samples (if graphics then it is displayed graphically aswell)

void printCanvas(int samples, int graphics, int num_cores) 
// effectively prints the workstation for printUsage, gives samples amount of space for data and if graphics gives space for graphics
// prints out the number of course (num_cores) in the correct location

float getUsage(char *extra)
// Uses /proc/stat to get cpu_usage data, needs extra to be passed as extra space for the data inside of the proc file
// calculates CPU usage by dividing the amount of idle by total (giving the decimal of time idle)
// then  1 - decimal of time idle = decimal of time not idle
// multiply that number by 100 to get the percentage of cpu not idle or the percentage being used

int getCores() 
// reads /proc/cpuinfo to get and return the number of cores your cpu has

void cpuGraphic(float delta) 
// prints a '|' character to visualize every 0.01 of CPU usage (delta) and prints delta at the end

void printUsage(int graphics, int sequential, int samples, float tdelay, int users)
// this function does a bulk of the processing and output of CPU and memory usage
// reads from /proc/meminfo to get total physical and virtual memory, uses that to output desired data
// uses graphics, sequential, and users as criteria for display if any of them are true it alters the display process
// gets sample data based on samples samples waiting between tdelay seconds between each
// heavy documentation on the code of this function, lots to track

void printUsers(int samples, float tdelay, int sequential) 
// for when '--user' flag is passed to the terminal, this function outputs the proper data required for that flag
// gets sample data based on samples samples waiting between tdelay seconds between each
// if sequential then the function alters its display

int main(int argc, char **argv) 
// parses all the arguments passed through the terminal, directs each to the location that provides the correct output (i.e. acts as 
// a menu)

	how to run (use) your program
Accepts the following arguments:
--system 	: solely displays system data (no user data)
--user		: solely displays user data (no system data)
--graphics 	: provides visualization for CPU and memory data
-g		: equivalent to --graphics
--sequential	: changes the output to be sequential (i.e. instead of live updates, prints one after another)
--samples=#	: replace # with an integer to change the number of data points taken
--tdelay=#	: replace # with a float to change the amount of time (in seconds) between each sampling
The final 2 arguments may also be passed as positional arguments in the order 'int float', 
this will be read as '--samples=int --tdelay=float' 

If duplicate arguments are passed, the last instance of the argument will be considered (i.e. two calls to --samples=# will override
eachother), if neither or both --system and --user are passed then both sets of information will be output. It is never needed to
pass the same argument multiple times.

If unreasonable (considering typical) amounts of users are connected, CPU is being used, or memory is being used occassionally 
the terminal will not provide enough space for output and graphics may overlap/spill into other lines.
In this case, pass the 'clear' argument to your terminal after the program terminates and attempt to run the program again.
(This might also occur if the terminal is atypically small, or a number of other abnormal conditions)