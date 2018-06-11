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

/**
 * Create a sempahore
 * @param  ipc_key key_t value for semaphore key generation
 * @return         sempahore identifier, otherwise -1 is returned
 */
int createsem (key_t ipc_key);

/**
 * Set a value for a semaphore
 * @param  sem_id  semaphore identifier
 * @param  sem_num semaphore number
 * @param  value   initial value
 * @return         on failure returns -1 with errno indicating the error,
 *                    otherwise a nonnegative value
 */
int setsemval (int sem_id, int sem_num, int value);

/**
 * Get a semaphore value
 * @param  sem_id semaphore identifier
 * @return        returns the value of semval
 */
int getsemval (int sem_id);

/**
 * Lock a semaphore
 * @param  sem_id  semaphore identifier
 * @param  sem_num semaphore number
 * @return         0 successful otherwise they return -1 with errno indicating the error.
 */
int locksem (int sem_id, int sem_num);

/**
 * Unlock a semaphore
 * @param  sem_id  semaphore identifier
 * @param  sem_num semaphore number
 * @return         0 successful otherwise they return -1 with errno indicating the error.
 */
int unlocksem (int sem_id, int sem_num);

/**
 * Remove the semaphore
 * @param  sem_id semaphore identifier
 * @return        on failure returns -1 with errno indicating the error,
 *                    otherwise a nonnegative value
 */
int removesem (int sem_id);

#endif /* SEMAPHORE_H */
