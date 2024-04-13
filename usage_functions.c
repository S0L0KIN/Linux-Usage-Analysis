#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h> //is for usage in header
#include <sys/utsname.h> //is for info in sysinfo
#include <unistd.h> //for sleep/wait function
#include <utmp.h> //is for user info
#include "display.h"
#include "usage_functions.h"
#define TRUE 1
#define FALSE 0
#define MAX_STR_LENGTH 1024

/*
 * @brief sleeps the system for a float amount of time
 * @param tdelay amount of time to sleep for
 */
void wait(float tdelay) {
    int seconds = (int)tdelay; //cast down to seconds
    tdelay -= seconds; //left only with milliseconds
    sleep(seconds); //sleep for all the seconds
    usleep(tdelay*1000000); //sleep for the milliseconds (converted to microseconds)
}

/*
 * @brief counts how many users are connected at a given time
 * @return the number of connected users
 */
int countConnectedUsers() { //have to split count from print as we need to return the string from the pipe,
    int user_count = 0; //create countConnectedUsers so we can still newline a canvas for our code
    
    FILE *user_file = NULL;
    user_file = fopen(UTMP_FILE, "r");
    if (user_file == NULL) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }

    struct utmp user_entry;
    while (fread(&user_entry, sizeof(struct utmp), 1, user_file) == 1) {
        if (user_entry.ut_type == USER_PROCESS) {
            user_count++;
        }
    }

    fclose(user_file);
    return user_count;
}

/*
 * @brief gets connected user information
 * @return string of connected user information
 */
char* getUsersInfo() { //same as old print users info but returns it as a string
    char* buffer = (char*)malloc(MAX_STR_LENGTH * sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }
    snprintf(buffer, MAX_STR_LENGTH, "### Sessions/Users ###\n");
    
    FILE *user_file = NULL;
    user_file = fopen(UTMP_FILE, "r");
    if (user_file == NULL) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }

    struct utmp user_entry;
    while (fread(&user_entry, sizeof(struct utmp), 1, user_file) == 1) {
        if (user_entry.ut_type == USER_PROCESS) {
            char line[MAX_STR_LENGTH];
            snprintf(line, MAX_STR_LENGTH, "%s\t%s (%s)", user_entry.ut_user, user_entry.ut_line, user_entry.ut_host);
            strncat(buffer, line, MAX_STR_LENGTH - strlen(buffer) - 1);
        }
    }

    fclose(user_file);

    return buffer;
}

/*
 * @brief computes memory usage at time of calling
 * @param graphics denotes whether graphical visualization of change should be appended
 * @param prev_memory pointer to float containing previous memory usage
 * @return string with memory information + optional graphical visualization (if graphics)
 */
char *getMemDiff(int graphics, float *prev_memory) {
    FILE *memory_file = fopen("/proc/meminfo", "r");
    if (memory_file == NULL) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }
    

    char line[MAX_STR_LENGTH];
    float total_swap = 0;
    float free_swap = 0;
    float total_phymem = 0;
    float free_phymem = 0;
    float buffers = 0;
    float cached = 0;

    while (fgets(line, MAX_STR_LENGTH, memory_file))
    {
        if (strncmp(line, "SwapTotal", 9) == 0) {
            sscanf(line, "SwapTotal: %f kB", &total_swap);
            total_swap = total_swap;
        }
        if (strncmp(line, "SwapFree", 8) == 0) {
            sscanf(line, "SwapFree: %f kB", &free_swap);
            free_swap = free_swap;
        }
        if (strncmp(line, "MemTotal", 8) == 0) {
            sscanf(line, "MemTotal: %f kB", &total_phymem);
            total_phymem = total_phymem;
        }
        if (strncmp(line, "MemFree", 7) == 0) {
            sscanf(line, "MemFree: %f kB", &free_phymem);
            free_phymem = free_phymem;
        }
            if (strncmp(line, "Buffers", 7) == 0) {
            sscanf(line, "Buffers: %f kB", &buffers);
        buffers = buffers;
        }
        if (strncmp(line, "Cached", 6) == 0) {
            sscanf(line, "Cached: %f kB", &cached);
            cached = cached;
        }
    }

    double total_virmem = total_phymem + total_swap;
    double virmem_used = total_virmem - free_phymem - buffers - cached - free_swap;

    // Constructing the output string
    char output[1024];
    snprintf(output, sizeof(output), "%.2f GB / %.2f GB -- %.2f GB / %.2f GB", (total_phymem - free_phymem) / 1000000, total_phymem / 1000000, virmem_used / 1000000, total_virmem / 1000000);

    if (graphics) {
        float memdiff = (virmem_used - *prev_memory) / 1000000;
        strcat(output, "\t|");
        if (*prev_memory == 0 || memdiff == 0) {
            strcat(output, "o (");
            char memdiff_str[20];
            snprintf(memdiff_str, sizeof(memdiff_str), "%.2f", virmem_used / 1000000);
            strcat(output, memdiff_str);
            strcat(output, ")");
        } else if (memdiff > 0) {
            for (float i = 0; i < memdiff; i += 0.03) {
                strcat(output, "#");
            }
            strcat(output, "* ");
            char memdiff_str[20];
            snprintf(memdiff_str, sizeof(memdiff_str), "%.2f", memdiff);
            strcat(output, memdiff_str);
            strcat(output, " (");
            snprintf(memdiff_str, sizeof(memdiff_str), "%.2f", virmem_used / 1000000);
            strcat(output, memdiff_str);
            strcat(output, ")");
        } else {
            for (float i = 0; i > memdiff; i -= 0.03) {
                strcat(output, ":");
            }
            strcat(output, "@ ");
            char memdiff_str[20];
            snprintf(memdiff_str, sizeof(memdiff_str), "%.2f", memdiff);
            strcat(output, memdiff_str);
            strcat(output, " (");
            snprintf(memdiff_str, sizeof(memdiff_str), "%.2f", virmem_used / 1000000);
            strcat(output, memdiff_str);
            strcat(output, ")");
        }
    }
    //strcat(output, "\n");
    fclose(memory_file);

    *prev_memory = virmem_used;

    char *ret_str = (char *)malloc(MAX_STR_LENGTH);
    strcpy(ret_str, output);
    return ret_str;
}

/*
 * @brief counts the number of cpu cores
 * @return the number of cores
 */
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

/* 
 * @brief calculates cpu usage over time interval
 * @param tdelay length of that interval
 * @return the cpu usage
 */
float getUsage(float tdelay) {
    FILE *stat_file = NULL; //open file and check if valid
    stat_file = fopen("/proc/stat", "r");
    if (stat_file == NULL) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }

    float user = 0;
    float nice = 0;
    float system = 0; 
    float idle = 0;
    float iowait = 0; 
    float irq = 0;
    float softirq = 0;
    float total1 = 0;
    float used1 = 0;
    char raw_data[1024] = {'\0'}; 

    if (!fgets(raw_data, sizeof(raw_data), stat_file)) {
        perror("Could not read from file.");
        fclose(stat_file);
        exit(1);
    }

    sscanf(raw_data, "cpu %f %f %f %f %f %f %f", &user, &nice, &system, &idle, &iowait, &irq, &softirq);
    total1 = user + nice + system + idle + iowait + irq + softirq;
    used1 = total1 - idle;

    rewind(stat_file);
    wait(tdelay);
    
    float total2 = 0;
    float used2 = 0;
    if (!fgets(raw_data, sizeof(raw_data), stat_file)) {
        perror("Could not read from file.");
        fclose(stat_file);
        exit(1);
    }
    sscanf(raw_data, "cpu %f %f %f %f %f %f %f", &user, &nice, &system, &idle, &iowait, &irq, &softirq);
    total2 = user + nice + system + idle + iowait + irq + softirq;
    used2 = total2 - idle;

    fclose(stat_file);
    return ((float)(used2 - used1) / (float)(total2 - total1)) * 100.0; //cpu usage calculation
}

