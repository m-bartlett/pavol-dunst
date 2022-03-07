#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdbool.h>

int get_shmid(char *shm_path, int project_id);
bool process_mutex_lock(int shmid);
int process_mutex_unlock(int shmid);