#include "sharedmem.h"

void *
createshm (key_t ipc_key, int len, int *shm_id)
{
    *shm_id = shmget(ipc_key, len, SHM_FLAG);

    if (*shm_id == -1)
        return NULL;

    return shmat(*shm_id, 0, 0);
}

void *
getshm (key_t ipc_key, int *shm_id)
{
    *shm_id = shmget(ipc_key, 0, IPC_EXCL);

    if (*shm_id < 0)
        return NULL;

    return shmat(*shm_id, 0, 0);
}

int
detachshm (const void *shmaddr)
{
    return shmdt(shmaddr);
}

int
removeshm (int shm_id)
{
    return shmctl (shm_id, IPC_RMID, NULL);
}

