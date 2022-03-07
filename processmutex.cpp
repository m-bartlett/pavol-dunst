#include "processmutex.h"

int get_shmid(char *shm_path, int project_id) {
    key_t key = ftok(shm_path, project_id);
    int shmid = shmget(key,1024,0666|IPC_CREAT);
    return shmid;
}

bool process_mutex_lock(int shmid) {
    void *shmptr = shmat(shmid,(void*)0,0);
    char *shmdata = (char*)shmptr;
    if (shmdata[0] == 1 || shmdata[0] == -1) return false;
    shmdata[0] = 1;
    shmdt(shmptr);
    return true;
}

int process_mutex_unlock(int shmid) {
    int status = shmctl(shmid, IPC_RMID, NULL);
    return status;
}


/*
#include "processmutex.h"

#include <unistd.h> // usleep
#include <stdlib.h> // EXIT_*

#define SHARED_MEMORY_PATH_PREFIX "shm_"
#define PROJECT_ID 93   // for ftok()

int main(int argc, char *argv[]) {

  size_t shmpath_size = strlen(argv[0]) + strlen(SHARED_MEMORY_PATH_PREFIX);
  char shmpath[shmpath_size];
  strcat(shmpath, SHARED_MEMORY_PATH_PREFIX);
  strcat(shmpath, argv[0]);
  printf("%s\n", shmpath);

  int shmid = get_shmid(shmpath, PROJECT_ID);
  if (shmid == -1) {
    perror("Error retreiving shared memory id");
    return EXIT_FAILURE;
  }

  bool process_locked = process_mutex_lock(shmid);

  if (!process_locked) {
    fprintf(stderr, "Error locking process mutex\n");
    exit(EXIT_FAILURE);
  }

  usleep(10*1000*1000);

  int process_mutex_unlock_status = process_mutex_unlock(shmid);
  if (process_mutex_unlock_status == -1) {
    perror("Error unlocking process mutex");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
*/