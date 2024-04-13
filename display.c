#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h> //is for usage in header
#include <sys/utsname.h> //is for info in sysinfo
#include <unistd.h> //for sleep/wait function
#include <utmp.h> //is for user info

#define TRUE 1
#define FALSE 0
#define MAX_STR_LENGTH 1024

/*
 * @brief prints row of -'s as a divider
 */
void printDivider() { //just prints the divider, more legible shorthand than writing the line each time
    printf("---------------------------------------\n");
}

/*
 * @brief prints the header portion of the output
 * @param samples amount of samples to be taken
 * @param frequency amount of time between each sample
 */
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

/*
 * @brief prints the system info at the footer of the visualization
 */
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

/*
 * @brief prints the static outline leaving space for the dynamic data to be filled in
 * @param samples amount of samples to be taken
 * @param graphics denotes whether or not graphics should be printed
 * @param num_cores number of cores the CPU has 
 * @param users denotes whether or not users should be printed
 */
void printCanvas(int samples, int graphics, int num_cores, int users) { 
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
    for(int i=0; i<samples; i++) { //space for mem usage
        printf("\n");
    }
    printDivider();
    if (users) { //space for users
        for (int i=0; i<users+1; i++) {
            printf("\n");
        }
        printDivider();
    }
    printf("number of cores: %d\n", num_cores); //print cores
    printf("\n");//THIS LINE IN SPECIFIC IS TO BE OVERWRITTEN WITH 'total cpu use = %d\n'
    if (graphics) {
        for(int i=0; i<samples; i++) { //if graphics then need extra space for CPU visualization
            printf("\n");
        }
    }
    printDivider();
}

/*
 * @brief visualizes the amount of cpu usage with |'s
 * @param delta current usage
 */
void cpuGraphic(float delta) {
    for(float i=0; i<delta; i+=0.5) {
        printf("|");
    }
    printf(" %.2f\n", delta);
}

