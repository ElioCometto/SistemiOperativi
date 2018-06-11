#include "semaphore.h"

int
createsem (key_t ipc_key)
{
    /**
     * Create new set of semaphores of length NSEMS
     * IPC_CREAT: If sem not exists, create a new one
     * IPC_EXCL: Return a EEXIST failure if a set with the ipc_key already
     * exists
     */
    return semget(ipc_key, N_SEMS, SEM_FLAG);
}

int
setsemval (int sem_id, int sem_num, int value)
{
    union semun arg;

    arg.val = value;   /* initial argument value */

    /**
     * Set the value of semval to arg.val for the semnum-th semaphore of the set,
     * updating also the sem_ctime member of the semid_ds structure associated with the set.
     * Undo entries are cleared for altered semaphores in all processes.
     * If the changes to semaphore values would permit blocked semop(2) calls in other processes to proceed,
     * then those processes are woken up. The calling process must have alter permission on the semaphore set.
     */
    return semctl(sem_id, sem_num, SETVAL, arg);
}

int
getsemval (int sem_id)
{
    /**
     * The system call returns the value of semval for the semnum-th semaphore of the set.
     * The calling process must have read permission on the semaphore set.
     */
    return  semctl(sem_id, 0, GETVAL);
}

int
locksem (int sem_id, int sem_num)
{
    struct sembuf sem_lock = {
        sem_num,
        0,
        SEM_UNDO
    };

    /**
     * semop() performs operations on selected semaphores in the set indicated by semid.
     * Each of the nsops elements in the array pointed to by sops specifies an operation
     * to be performed on a single semaphore. The elements of this structure are of type struct sembuf,
     * containing the following members:
     *     unsigned short sem_num;  // semaphore number
     *     short          sem_op;   // semaphore operation
     *     short          sem_flg;  // operation flags
     * Flags recognized in sem_flg are IPC_NOWAIT and SEM_UNDO.
     * If an operation specifies SEM_UNDO, it will be automatically undone when the process terminates.
     */
    return semop(sem_id, &sem_lock, 1);
}

int
unlocksem (int sem_id, int sem_num)
{
    struct sembuf sem_unlock = {
        sem_num,
        1,
        SEM_UNDO
    };

    /**
     * semop() performs operations on selected semaphores in the set indicated by semid.
     * Each of the nsops elements in the array pointed to by sops specifies an operation
     * to be performed on a single semaphore. The elements of this structure are of type struct sembuf,
     * containing the following members:
     *     unsigned short sem_num;  // semaphore number
     *     short          sem_op;   // semaphore operation
     *     short          sem_flg;  // operation flags
     * Flags recognized in sem_flg are IPC_NOWAIT and SEM_UNDO.
     * If an operation specifies SEM_UNDO, it will be automatically undone when the process terminates.
     */
    return semop(sem_id, &sem_unlock, 1);
}

int
removesem (int sem_id)
{
    /**
     * Immediately remove the semaphore set,
     * awakening all processes blocked in semop(2)
     * calls on the set (with an error return and errno set to EIDRM).
     * The effective user ID of the calling process must match the creator or owner of the semaphore set,
     * or the caller must be privileged. The argument semnum is ignored.
     */
    return semctl(sem_id, 0, IPC_RMID);
}

