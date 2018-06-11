#ifndef SHAREDMEM_H
#define SHAREDMEM_H

#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>

//#define SHM_FLAG    (IPC_CREAT | IPC_EXCL | 0666)
#define SHM_FLAG    (IPC_CREAT | 0666)

void *createshm (key_t ipc_key, int len, int *shm_id);

void *getshm (key_t ipc_key, int *shm_id);

int detachshm (const void *shmaddr);

int removeshm (int shm_id);

#endif  /* SHAREDMEM_H */
