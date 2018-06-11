#ifndef LIBRARY_H
#define LIBRARY_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define KEY_MEMORIA 5555
#define KEY_SEM 8888
#define SHM_SPACE sizeof(unsigned long) * 3
#define strlens(s) (s==NULL?0:strlen(s))
#define KEY_MSGQ 9999


key_t shm_key = 5555;

/* struttura fondamentale che memorizza i dati dell'individuo */
typedef struct Individuo{
  char tipo[2];
	char* name;
	unsigned long genoma;
	pid_t pid;
} _individuo;
typedef _individuo* individuo;

/*  struttura per l'invio di messaggi */
struct msgbuf{
  long mtype;
  unsigned long mtext[2];
};


/* Permette di deallocare la memoria usata dalle malloc della struct _individuo */
void pulisci_persona (individuo p){
  free((void*) p->name);
  free((void*) p);
}


/*  Esegue la stampa  di tutti gli attributi di un individuo  */
void debug_individuo(individuo test){
  if(test->pid != 0){
    printf("Genere : %s\n", test->tipo);
    printf("Nome: %s\n", test->name);
    printf("Genoma: %lu\n", test->genoma);
    printf("Pid: %lu\n\n", (unsigned long)test->pid);
  }else{
    printf("Individuo inesistente\n");
  }
}

void test_stampa_individuo(_individuo test){
  /*int sem_id = semget(KEY_SEM, 0, 0);
  getsemval(sem_id);
  locksem(sem_id, 0);*/
  
  if(test.pid != 0){
    printf("Genere : %s\n", test.tipo);
    printf("Nome: %s\n", test.name);
    printf("Genoma: %lu\n", test.genoma);
    printf("Pid: %lu\n\n", (unsigned long)test.pid);
  }
  //unlocksem(sem_id, 0);
}

void print_shm(individuo pshm, int individui){
  int i;
  
  for(i = 0; i < individui; i++){
    //debug_individuo(pshm[i]);
    test_stampa_individuo(pshm[i]);
  }
}

char* unslong_to_string(unsigned long ul){
  int n = snprintf(NULL, 0, "%lu", ul);
  char* buf = (char *) malloc(n+1);
  int c = snprintf(buf, n+1, "%lu", ul);
  
  return buf;
}


void (*sighandler(int signum)){
  printf("sono sighandler\n");
}

#endif 
