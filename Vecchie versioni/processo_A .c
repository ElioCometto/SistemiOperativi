#include <stdio.h>
#include "library.h"
#include "sharedmem.h"
#include "semaphore.h"

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
  pers->name[strlens(name)] = '\0';
  pers->genoma = gen;
  
  pers->pid = getpid();
  
  return pers;
}

void scrivi_info(individuo me, unsigned long *meshm){
  int i = 0;
  
  while(i < (sizeof(meshm) / SHM_SPACE)){
    if(meshm[i * SHM_SPACE] == 0){
      meshm[i * SHM_SPACE] = 1;
      meshm[i * SHM_SPACE + sizeof(unsigned long)] = me->genoma;
      meshm[i * SHM_SPACE + sizeof(unsigned long) * 2] = getpid();
    }
  }
}


int main(int argc, char* argv[]) {
  individuo me = initialize_individuo(argv[0], strtoul(argv[1], NULL, 10));
  int shm_id, i;
  void *shm;
  int sem_id, msgq_id;

  sem_id = semget(KEY_SEM, 0, 0);
  getsemval(sem_id);
  
  printf("Sono il processo_A\n");
  debug_individuo(me);
  
  shm_id = shmget(shm_key, 0, 0);
  shm = (int *) shmat(shm_id, NULL, 0);
  if(shm == NULL){
    printf("Errore nella creazione della memoria condivisa\n");
    exit(EXIT_FAILURE);
  }
  
  scrivi_info(me, shm);
  
  //print_shm(shm);
  
  //Crea la coda dei messaggi 
  msgq_id = msgget(me.pid, IPC_CREAT | 0666);
  if(msgq_id == -1) {
    printf("Errore nel creare la coda dei messaggi\n");
    exit(EXIT_FAILURE);
  } 
  //Leggi dalla coda(processo in wait)
  if(msgrcv(msgq_id, &msgp,sizeof(individuo), 0,0) == -1){
    printf("Errore nel ricevere info di B\n");
    exit(EXIT_FAILURE);
  }
  //Valuta informazioni di B 
  va_bene = valuta_info_B(msgp.individuo); // TODO: implementa valuta info B
  risp.va_bene = va_bene;
  risp.mtype = 1;
  if (va_bene){
  //Scrivi su coda di messaggi OK(verso processo B)
    if(msgsnd(msgq_id, &risp,sizeof(int), 0) == -1){
      printf("Errore nel rispondere al processo B\n");
      exit(EXIT_FAILURE);
    }
  //Scrivi su coda di messaggi Pid di B (verso il gestore)
    nome_messaggio.pid = msgp.individuo.pid;
    nome_messaggio.mtype = 1;
    if(msgsnd(TODO_KEY, &nome_messaggio, sizeof(int), 0) == -1){
      printf("Errore nel informare il gestore\n");
      exit(EXIT_FAILURE);
    }
  //Termina liberando risorse 
    if(msgctl(msgq_id, IPC_RMID, NULL) == -1){
      printf("Errore nella chiusura della coda di messaggi\n");
      exit(EXIT_FAILURE);
    }
    if(releaseSem(KEY_SEM, 0) == -1){ //TODO: scrivere funzione release SEM
      printf("Errore nel rilasciare il semaforo\n");
      exit(EXIT_FAILURE);
    } 
    if(shmdt(sharedmem) == -1){
      printf("Errore nel staccarsi dalla memoria condivisa\n");
      exit(EXIT_FAILURE);
    }
  } else {
  //Scrivi su coda di messaggi NO(verso processo B)
    if(msgsnd(msgq_id, &risp,sizeof(int), 0) == -1){
      printf("Errore nel rispondere al processo B\n");
      exit(EXIT_FAILURE);
    }
  //if ho rifiutato tutti B allora 
  //Abbassa il target
    
  }



}

