/* store all three child ids statically (do not need to change)
 * 0 - memory fork
 * 1 - cpu fork
 * 2 - user fork
 */
static int pids[3] = {0, 0, 0};

/*
 * @brief sets process id into array
 * @param index is the index of insertion
 * @param pid is the childs process id
 */
void set_pid(int index, int pid) {
    pids[index] = pid;
}

/*
 * @brief gets process id at given spot
 * @param index is the desired pid  in array
 * @return the process id  
 */
int get_pid(int index) {
    return pids[index];
}