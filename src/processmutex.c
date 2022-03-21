#include "processmutex.h"

// Note: these are completely arbitrary values
#define SHARED_MEMORY_PATH_PREFIX "shm_pavol-dunst"
#define SHARED_MEMORY_PROJECT_ID  93

// static const int SHMID = shmget( ftok(SHARED_MEMORY_PATH_PREFIX, SHARED_MEMORY_PROJECT_ID),
//                                  1024,
//                                  0666|IPC_CREAT);


void process_mutex_lock() {
  int SHMID = shmget( ftok(SHARED_MEMORY_PATH_PREFIX, SHARED_MEMORY_PROJECT_ID),
                                 1024,
                                 0666|IPC_CREAT);
  if (SHMID == -1) { perror("Error retreiving shared memory id");
                     exit(EXIT_FAILURE); }
  void *shmptr = shmat(SHMID, (void*)0, 0);
  char *shmdata = (char*)shmptr;
  if (shmdata[0] == 1 || shmdata[0] == -1) {
    fprintf(stderr, "Process mutex locked, dying.\n");
    exit(EXIT_FAILURE);
  }
  shmdata[0] = 1;
  shmdt(shmptr);
}


void process_mutex_unlock() {
  int SHMID = shmget( ftok(SHARED_MEMORY_PATH_PREFIX, SHARED_MEMORY_PROJECT_ID),
                                 1024,
                                 0666|IPC_CREAT);
  if (shmctl(SHMID, IPC_RMID, NULL) == -1) {
    perror("Error unlocking process mutex");
    exit(EXIT_FAILURE);
  }
}