#include <stdio.h>
#include <stdlib.h>
#include "library.h"
#include "sharedmem.h"
#include "semaphore.h"

int sem_id;

individuo initialize_individuo(char* name, unsigned long gen){
  int i;
  individuo pers = (individuo) malloc(sizeof(individuo));
  pers->name = (char *) malloc(sizeof(char) * (strlens(name) + 1));
  
  if(pers->name == NULL){
    printf("Error: can't initialize the struct in individuo_A");
    exit(EXIT_FAILURE);
  }
  
  pers->tipo[0] = 'A';
  pers->tipo[1] = '\0';
  for(i = 0; i < strlens(name); i++){
    pers->name[i] = name[i];
  }
  pers->name[i + 1] = '\0';
  //pers->name[strlens(name)] = '\0';
  pers->genoma = gen;
  
  pers->pid = getpid();
  
  return pers;
}

void scrivi_info(individuo me, individuo meshm){
  int i = 0, flag = 0;
  getsemval(sem_id);
  locksem(sem_id, 0);
  
  while(i < 5 && flag == 0){    
    if(meshm[i].pid == 0){      
      strcpy(meshm[i].tipo, me->tipo);
      //meshm[i].name = (char*) malloc(sizeof(char) * strlens(me->name));
      //strcpy(meshm[i].name, me->name);
      meshm[i].genoma = me->genoma;      
      meshm[i].pid = getpid();
      //meshm[i] = me; 
      flag = 1; 
    }
    i++;
  }
  unlocksem(sem_id, 0);
}


int main(int argc, char* argv[]) {
  individuo me = initialize_individuo(argv[0], strtoul(argv[1], NULL, 10));
  int shm_id, i;
  void *shm;
  
  sem_id = semget(KEY_SEM, 0, 0);
  getsemval(sem_id);
  
  printf("Sono il processo_A\n");
  debug_individuo(me);
  
  shm_id = shmget(shm_key, 0, 0);
  shm = (unsigned long *) shmat(shm_id, NULL, 0);
  
  if(shm == NULL){
    printf("Errore nella creazione della memoria condivisa\n");
    exit(EXIT_FAILURE);
  }
  
  scrivi_info(me, shm);
}

