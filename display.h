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