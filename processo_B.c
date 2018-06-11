#include <stdio.h>
#include "library.h"
#include "sharedmem.h"
#include "semaphore.h"

#define DATA 4
#define MAX_LENGHT 20

int init_people;
int msgq_id;
struct msgbuf *msgp;
void *puntatore_shm;
individuo me;

void libera_risorse(){
	if(msgctl(msgq_id, IPC_RMID, NULL) == -1){
      printf("Errore nella chiusura della coda di messaggi\n");
      exit(EXIT_FAILURE);
    }
    if(shmdt(puntatore_shm) == -1){
      printf("Errore nel staccarsi dalla memoria condivisa\n");
      exit(EXIT_FAILURE);
    }
    pulisci_persona(me);
    exit(1);
}

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

unsigned long MCD(unsigned long genB, unsigned long genA){
	unsigned long r; //resto
	unsigned long a = genA;
	unsigned long b = genB; 

	 while(b != 0){
	 	r = a % b;
	 	a = b;
	 	b = r;
	 }
	 
	 return a;
}

int comparator(unsigned long gen1, unsigned long gen2){
  if(MCD(me->genoma, gen1) > MCD(me->genoma, gen2))
    return 1;
  else
    return -1;  
}

void invia_messaggio(unsigned long stato, pid_t pid_destinatario, pid_t pid_oggetto){
	struct msgbuf sbuf;

	sbuf.mtype = pid_destinatario;
	sbuf.mtext[0] = stato; // stato di accetatto (1), rifiutato(0)
	sbuf.mtext[1] = pid_oggetto;

	  //Scrivi su coda di messaggi OK(verso processo B)
    if(msgsnd(msgq_id, &sbuf, sizeof(sbuf),0)){
      printf("Errore nel rispondere al processo B\n");
      exit(EXIT_FAILURE);
    }
}

void ordina_array(individuo *arrA, int numA){
  int i, j;
  individuo tmp;
  int posmax;
  
  for(i = 0; i < numA; i++){
    posmax = i;
    for (j = (i + 1); j < numA; j++){
      if(comparator(arrA[j]->genoma , arrA[posmax]->genoma) > 0)
        posmax = j;
    }
    if(posmax != i){
      tmp = arrA[i];
      arrA[i] = arrA[posmax];
      arrA[posmax] = tmp;
    }
  }
}

int leggi_messaggio(){
  ssize_t nbyte = msgrcv(msgq_id, msgp, sizeof(struct msgbuf), me->pid, 0);
  
  return nbyte;
}

void contatta_processo_A(individuo *arrA, int numA){
  int i;
  
  while(i < numA){
    invia_messaggio(me->genoma, arrA[i]->pid, me->pid);
    sleep(0.25);
    while(leggi_messaggio() <= 0){
      sleep(0.1);
    }
    if(leggi_messaggio > 0){
      invia_messaggio(getpid(), getppid(), arrA[i]->pid);
      libera_risorse();
    }
    i++;
  }
}

void scegli_A(individuo shm){
  /*leggo da file quanti processi ci sono per allocare un array della lunghezza giusta*/
  int i,j, numA;
  char *shptr;
  individuo *arrA;
  
  arrA = (individuo *) malloc(sizeof(individuo) * init_people);

  numA = 0;
  for(i = 0; i < init_people; i++){
    if(shm[i].pid != 0){
      arrA[numA]->genoma = shm[i].genoma;
      arrA[numA]->pid = shm[i].pid;
      numA++;
    }
  }

  ordina_array(arrA, numA);
  
  contatta_processo_A(arrA, numA);
  
  free(arrA);
}

void leggi_file(){
  FILE *fp = fopen("config.txt", "r");
  char *line_buffer = (char *) malloc(sizeof(char) * MAX_LENGHT);
  size_t buffer_size = MAX_LENGHT;
  int data[DATA];
  int i;
  
  getline(&line_buffer, &buffer_size, fp);
  
  for(i = 0; i < DATA; i++){
    data[i] = atoi(strsep(&line_buffer, ";"));
  }

  init_people = data[0];
  fclose(fp);
}

void (*handler(int sig)){
	if (sig == SIGTERM){
		libera_risorse();
	}
}

int main(int argc, char* argv[]) {
  me = initialize_individuo(argv[0], strtoul(argv[1], NULL, 10));
  int shm_id, sem_id;

  leggi_file();
  sem_id = semget(KEY_SEM, 0, 0);
  getsemval(sem_id);
  
  printf("Sono il processo_B\n");
  
  msgq_id = msgget(KEY_MSGQ,0);
  
  shm_id = shmget(shm_key, 0, 0);
  puntatore_shm = (individuo) shmat(shm_id, NULL, 0);
  if(puntatore_shm == NULL){
    printf("Errore nella lettura della memoria condivisa\n");
    exit(EXIT_FAILURE);
  }
  
  while(1){
    scegli_A(puntatore_shm);
    sleep(0.5);
  }
}

