#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>

#define N_SEMS      1                          /* default number of semaphores */
#define SEM_FLAG    (IPC_CREAT|IPC_EXCL|0666)   /* default flag for semget */

/**
 * union semun type for fourth argument of semctl
 */
union semun {
  int              val;   /* value for SETVAL */
  struct semid_ds *buf;   /* buffer for IPC_STAT, IPC_SET */
  unsigned short  *array; /* array for GETALL, SETALL */ 

  #if defined(__linux__)
  struct seminfo *__buf;  /* buffer for IPC_INFO */
  #endif
};

int initSemAvailable(int semId, int semNum);

int initSemInUse(int semId, int semNum);

int reserveSem(int semId, int semNum);

int releaseSem(int semId, int semNum);

int getsemval(int semId, int semNum);

#endif /* SEMAPHORE_H */
