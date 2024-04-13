#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h> //is for usage in header
#include <sys/utsname.h> //is for info in sysinfo
#include <unistd.h> //for sleep/wait function
#include <utmp.h> //is for user info
#include "display.h"
#define TRUE 1
#define FALSE 0
#define MAX_STR_LENGTH 1024

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