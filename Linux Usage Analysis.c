#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h> //is for usage in header
#include <sys/utsname.h> //is for info in sysinfo
#include <unistd.h> //for sleep/wait function
#include <utmp.h> //is for user info
#define TRUE 1
#define FALSE 0
#define MAX_STR_LENGTH 64

void printDivider() { //just prints the divider, more legible shorthand than writing the line each time
    printf("---------------------------------------\n");
}

void printHeader(int samples, float frequency) {
    printf("Nbr of samples: %d -- every %.2f secs\n", samples, frequency);
    
    struct rusage cur_usage; //for self diagnostic memory data
    
    if (getrusage(RUSAGE_SELF, &cur_usage)) { //ensure we create the object
        perror("Unable to assess self usage, try again\n");
        exit(1);
    } 
    
    printf("Memory usage: %ld kilobytes\n", cur_usage.ru_maxrss);
    printDivider();
}

void printSysInfo() {
    printf("### System Information ###\n");
    
    struct utsname info;
    if (uname(&info)) { //make sure struct is valid
        perror("Unable to get system info, try again\n");
        exit(1);
    }

    printf("System Name = %s\nMachine Name = %s\nVersion = %s\nRelease = %s\nArchitecture = %s\n", info.sysname, info.nodename, info.version, info.release, info.machine);

    
    FILE *uptime_file = NULL; //open file and check if valid
    uptime_file = fopen("/proc/uptime", "r");
    if (uptime_file == NULL) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }
    //for reading and parsing file
    char str_uptime[20] = {'\0'};
    long int uptime = 0;
    fgets(str_uptime, 20, uptime_file);

    uptime = atoi(str_uptime); //want in the form of days hours minutes seconds so need to do conversions
    long int minutes = uptime/60;
    uptime -= 60*minutes;
    long int hours = minutes/60;
    minutes -= 60*hours;
    int days = hours/24;
                                            //with days                                  with hours
    printf("System running since last reboot: %d days %ld hours %ld minutes %ld seconds (%ld:%02ld:%02ld)\n",days, hours-(days*24), minutes, uptime, hours, minutes, uptime);

    fclose(uptime_file);

    printDivider();
}

void wait(float tdelay) {
    int seconds = (int)tdelay; //cast down to seconds
    tdelay -= seconds; //left only with milliseconds
    sleep(seconds); //sleep for all the seconds
    usleep(tdelay*1000000); //sleep for the milliseconds (converted to microseconds)
}

int printUsersInfo() {
    printf("### Sessions/Users ###\n");
    
    FILE *user_file = NULL; //open and check utmp file (from utmp.h library, for utmp struct)
    user_file = fopen(UTMP_FILE, "r");
    if (user_file == NULL) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }

    struct utmp user_entry; //library handles most of the work, just have to read the data and print each one
    int user_count = 0;
    while (fread(&user_entry, sizeof(struct utmp), 1, user_file) == 1) {
        if (user_entry.ut_type == USER_PROCESS) {
            printf("%s\t%s (%s)\n", user_entry.ut_user, user_entry.ut_line, user_entry.ut_host);
            user_count+=1; //increment user count (this will help with formatting the ANSI escape codes later)
        }
    }

    fclose(user_file);
    printDivider();
    return user_count; //use this return for formatting when updating user count repeatedly

}

float printMemDiff(int graphics, float prev_memory, float total_phymem, float total_virtmem) {
    //Plan for this code: given previous memory usage, open file get current memory usage and calculate difference, print all the differences (if graphics print graphics after) 
    FILE *memory_file = NULL; //open file and check if valid
    memory_file = fopen("/proc/meminfo", "r");
    if (memory_file == NULL) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }

    float phymem_free =0; //need variables we can parse strings into
    float virmem_free =0;
    char str_memfree[30] = {'\0'}; //need strings we can parse file into
    char str_swapfree[30] = {'\0'};

    fgets(str_memfree, 30, memory_file);
    fgets(str_memfree, 30, memory_file); //do it twice cause want second line of proc file
    for (int i=0; i<14; i++) {
        fgets(str_swapfree, 30, memory_file); //skip ahead in file to the next value we want
    }
    phymem_free = atof(&str_memfree[15]); //getting the values of memory
    virmem_free = atof(&str_swapfree[15]);
    float phymem_used = total_phymem - phymem_free; //total - free = used
    float virmem_used =  total_virtmem - virmem_free - phymem_free; //same as above
    printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB", phymem_used/1000000, total_phymem/1000000, virmem_used/1000000, total_virtmem/1000000); //print out converting kb to gb

    if (graphics) { //if graphics have to do visualization
        float memdiff = (virmem_used - prev_memory)/1000000; //want to represent the difference from previous memory usage to current memory usage
        printf("\t|");
        if (prev_memory == 0 || memdiff == 0) { //0 difference case (or first call to this function)
            printf("o (%.2f)", virmem_used/1000000);
        }
        else if (memdiff > 0) { //positive increase formatting
            for(float i=0; i<memdiff; i+=0.03) {
                printf("#");
            }
            printf("* %.2f (%.2f)", memdiff, virmem_used/1000000);
        }
        else { //negative decrease formatting
            for(float i=0; i>memdiff; i-=0.03) {
                printf(":");
            } 
            printf("@ %.2f (%.2f)", memdiff, virmem_used/1000000);
        }
    }
    printf("\n");

    fclose(memory_file);
    return virmem_used; //close and return the last memory used so we know the difference for next sample
}
 
void printCanvas(int samples, int graphics, int num_cores) {
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
    for(int i=0; i<samples; i++) {
        printf("\n");
        // cur_mem = printMemDiff(graphics, cur_mem, tdelay, total_phymem, total_virmem);
        // wait(3);
    }
    printDivider();
    printf("number of cores: %d\n", num_cores); 
    printf("\n");//THIS LINE IN SPECIFIC IS TO BE OVERWRITTEN WITH 'total cpu use = %d\n'
    if (graphics) {
        for(int i=0; i<samples; i++) { //if graphics then need extra space for CPU visualization
            printf("\n");
        }
    }
    printDivider();
}

float getUsage(char *extra) {
    FILE *stat_file = NULL; //open file and check if valid
    stat_file = fopen("/proc/stat", "r");
    if (stat_file == NULL) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }

    float total = 0;
    float idle = 0; 
    char raw_data[100] = {'\0'}; 
    fgets(raw_data, 100, stat_file);
    strcpy(raw_data, &raw_data[3]);
    for(int i=0; i<10; i++) {
        if(i==3) {
            idle = strtof(raw_data, NULL);
        }

        total += strtof(raw_data, &extra);
        strncpy(raw_data, extra, strlen(extra));
    }
    fclose(stat_file);
    // intuition of operation is idle/total = decimal of time being idle
    // 1 - decimal of idle time = decimal of time not idle
    return 100*(1-idle/total); //100(decimal of time not idle) = percent of time in use

}

int getCores() {
    FILE *cores_file = NULL; //open file and check if valid
    cores_file = fopen("/proc/cpuinfo", "r");
    if (cores_file == NULL) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }
    char str_cores[30] = {'\0'}; //define string for the cores to be parsed from

    for(int i=0; i<13; i++) {  //skip to line with the data we want
        fgets(str_cores, 30, cores_file);
    }

    fclose(cores_file);
    return atoi(&str_cores[12]); //parse and return
}

void cpuGraphic(float delta) {
    for(float i=0; i<delta; i+=0.01) {
        printf("|");
    }
    printf(" %.2f\n", delta);
}

void printUsage(int graphics, int sequential, int samples, float tdelay, int users) {
    float total_phymem = 0;//want to set these values only one time to minimize complexity, so get from file
    float total_virmem = 0;
    char str_phymem[30] = {'\0'};
    char str_swaptot[30] = {'\0'};

    FILE *memory_file = NULL; //open file and check if valid
    memory_file = fopen("/proc/meminfo", "r");
    if (memory_file == NULL) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }
    fgets(str_phymem, 30, memory_file); //pull total memory and virtual memory
    for (int i=0; i<14; i++) {
        fgets(str_swaptot, 30, memory_file);
    }
    total_phymem = atof(&str_phymem[15]);
    total_virmem = total_phymem + atof(&str_swaptot[15]);
    fclose(memory_file);

    float cur_mem = 0; 
    int num_cores = getCores();
    int user_count = 0;

    //THIS IS WHERE THE PRINTING BEGINS!
    
    if (!sequential) { //if not sequential we only need 1 canvas, print it before the loop to accomplish this
        printCanvas(samples, graphics, num_cores);
        if (graphics) printf("\033[%dA", 2+samples); //graphics adds samples rows + 2 to get to cpu usage (treating cpu usage like home base)
        else printf("\033[2A"); //if no graphics, only need to go up 2
        printf("\033[s"); //save this place
    }
    float usage = 0;
    char *extra = malloc(sizeof(char)*100); //need this for getUsage function, have to declare here for reuse/prevent memleak
    for(int i=0; i<samples; i++) {
        if(sequential) {    //allows us to create multiple canvases + headers if sequential
            printf("\033[25B"); //send all the way to bottom to ensure sequential printing
            printf("Iteration: %d\n", i+1); //track iteration #s, wont effect escape code travelling cause dont ever go this high
            printHeader(samples, tdelay); 
            printCanvas(samples, graphics, num_cores);
            if (graphics) printf("\033[%dA", 2+samples); //similar to non-sequential case
            else printf("\033[2A");
            printf("\033[s");
        }

        printf("\033[u"); //back to cpu usage
        printf("\033[K"); //and ensure it is clear (it should be, but incase of overlap want to make sure cpu usage is prioritized)
        usage = getUsage(extra); //updates every iteration, then see below
        printf("total cpu use = %.2f%%", usage); //print out current usage

        printf("\033[%dA", (2+samples)-i); //go up to current line of memory (2 takes above cores and dividers, samples above canvas, down by i to get to current line)
        printf("\033[K"); 
        printf("\033[100D"); //clear the line and go all the way left (should be there anyways w/o this check)
        cur_mem = printMemDiff(graphics, cur_mem, total_phymem, total_virmem); //print out the current line of the memory
        printf("\033[u"); //return to cpu usage line (home base)

        if(graphics) { //if graphics is requested we must visualize the cpu usage
            printf("\033[%dB", i+1); //from homebase we jump down 1 + the iteration to get to the current line
            printf("\033[K"); 
            printf("\033[100D"); //clear the line and go all the way left (should be there anyways w/o this check)
            cpuGraphic(usage/100); //use our cpu visualization function
            printf("\033[u"); //return to homebase
        }

        if (users) { //if users are requested we must print them
            printf("\033[%dB", samples+1);
            user_count = printUsersInfo();
        }
        
        if (sequential) {
            printf("\033[100B");
            printSysInfo();
        }
        wait(tdelay);
        if (users && !sequential&& i!=samples-1) { //if we are not sequential and have users, we must clean up after our print
            for(int j=0; j<user_count+1; j++) { // divider + the title message + all the users must be deleted
                printf("\033[A");
                printf("\033[K");
            }
            printf("\033[%dA", 2+samples);
            printf("\033[K");
            printf("\033[s");
        }
    }
    printf("\033[%dB", 2+user_count);
    free(extra);
}

void printUsers(int samples, float tdelay, int sequential) {
    int user_count = 0;
    for(int i=0; i<samples; i++) {
        if (sequential) {
            printf("iteration: %d\n", i+1);
            printHeader(samples, tdelay);
        }
        user_count = printUsersInfo();
        wait(tdelay);
        if(sequential) printSysInfo();
        else {    
            if (i!= samples-1) {     
                for(int j=0; j<user_count+2; j++) {
                    printf("\033[A");
                    printf("\033[K");
                }   
            }
        }        
    }
}

int main(int argc, char **argv) {
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

    if(!sequential) printHeader(samples, frequency);
    if(usageType == 2) {
        printUsers(samples, frequency, sequential);
    }
    else if (usageType == 1) {
        printUsage(graphics, sequential, samples, frequency, 0);
    }
    else{
        printUsage(graphics, sequential, samples, frequency, 1);
    }
    //printf("I make it here :'(\n");
    if(!sequential) printSysInfo();

    return 0;
}
