#ifndef PROCESSMUTEX_H
#define PROCESSMUTEX_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

void process_mutex_lock();
void process_mutex_unlock();

#endif