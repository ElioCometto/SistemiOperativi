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
#define KEY_MSGQ 9999
#define SHM_SPACE sizeof(unsigned long) * 3
#define strlens(s) (s==NULL?0:strlen(s))

//key_t shm_key = 5555;

/* struttura fondamentale che memorizza i dati dell'individuo */
typedef struct Individuo{
  char tipo[2];
	char* name;
	unsigned long genoma;
	pid_t pid;
} _individuo;
typedef _individuo* individuo;

/* struttura del messaggio che contiene lo stato (letto o non letto) e il genoma di chi invia */
typedef struct msg
{
  unsigned long data;
  pid_t pid;
}messaggio;

/*  struttura per l'invio di messaggi */
struct msgbuf{
  long mtype;
  messaggio m;
};
//struct msqid_ds buf;


/* Permette di deallocare la memoria usata dalle malloc della struct _individuo */
void pulisci_persona (individuo p){
  free((void*) p->name);
  free((void*) p);
}

void updatemsgqueue(int msgq_id){
  struct msqid_ds buf;
  
  buf.msg_qbytes = sizeof(messaggio) * 300;
  msgctl(msgq_id, IPC_SET, &buf);
}

void printmsgqueue(int msgq_id){
  struct msqid_ds buf;
  
  msgctl(msgq_id, IPC_STAT, &buf);  
  
  printf("Massimo numero di byte in coda: %lu\n\n", buf.msg_qbytes);
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
  
  snprintf(buf, n+1, "%lu", ul);
  
  return buf;
}

char **strsplit(const char* str, const char* delim, size_t* numtokens) {
    char *s = strdup(str);
    size_t tokens_alloc = 1;
    size_t tokens_used = 0;
    char **tokens = calloc(tokens_alloc, sizeof(char*));
    char *token, *strtok_ctx;
    for (token = strtok_r(s, delim, &strtok_ctx);
            token != NULL;
            token = strtok_r(NULL, delim, &strtok_ctx)) {
        if (tokens_used == tokens_alloc) {
            tokens_alloc *= 2;
            tokens = realloc(tokens, tokens_alloc * sizeof(char*));
        }
        tokens[tokens_used++] = strdup(token);
    }
    // cleanup
    if (tokens_used == 0) {
        free(tokens);
        tokens = NULL;
    } else {
        tokens = realloc(tokens, tokens_used * sizeof(char*));
    }
    *numtokens = tokens_used;
    free(s);
    return tokens;
}


/*void (*sighandler(int signum)){
  printf("sono sighandler\n");
}*/

#endif 
