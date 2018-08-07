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
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include "library.h"
#include "sharedmem.h"
#include "semaphore.h"

#define DATA 4
#define MAX_LENGHT 20

int init_people = 0;
int genes = 0;
int birth_death = 0;
int sim_time = 0;
int shm_id = 0;
int popolazione[2];
individuo* persone;
int popolazione_attuale;
struct msgbuf msgp_r;
individuo tmp_individuo;
char* nome;
unsigned long gen;

/**  
 *  Crea un individuo 
 */
individuo init_individuo(char* nome_padre, char* nome_madre, int x, int gene){
	char nome[strlens(nome_padre) + strlens(nome_madre) + 2];
	char nome_tmp;
	//unsigned long genoma;
  individuo persona = (individuo) malloc(sizeof(individuo));
  persona->name = (char*) malloc(sizeof(char) * (strlens(nome_padre) + strlens(nome_madre) + 2));
  int i = 0, j = 0;  

  if(persona == NULL){
    printf("Error: can't initialize the struct individuo");
    exit(EXIT_FAILURE);
  }
  
  if(rand() % 2){
    persona->tipo[0] = 'A';
    persona->tipo[1] = '\0';
  }else{
    persona->tipo[0] = 'B';
    persona->tipo[1] = '\0';
  }
  
  nome_tmp = 65 + (rand() % 26);
  
  for(i = 0; i < strlens(nome_padre); i++){
    nome[i] = nome_padre[i];
  }
  for(j = 0; j < strlens(nome_madre); j++){
    nome[i+j] = nome_madre[j];
  }
  nome[i + j] = nome_tmp;
  nome[i + j + 1] = '\0';

  strncpy(persona->name, nome, strlens(nome_padre) + strlens(nome_madre) + 2);
  persona->genoma = x + (rand() % (x + gene));
     
  return persona;
}

/* Permette di deallocare la memoria usata dalle malloc della struct _individuo */
void pulisci_persona (individuo p){
  //free(p->name);
  //free(p);
}

individuo crea_individuoA(){
  individuo persona = (individuo) malloc(sizeof(individuo));
  persona->name = (char*) malloc(sizeof(char) * 2);

  if(persona == NULL){
    printf("Error: can't initialize the struct individuo");
    exit(EXIT_FAILURE);
  }
  
  persona->tipo[0] = 'A';
  persona->tipo[1] = '\0';
  
  persona->name[0] = 'A';
  persona->name[1] = '\0';
  persona->genoma = 100;
     
  return persona;
}

individuo crea_individuoB(){
  individuo persona = (individuo) malloc(sizeof(individuo));
  persona->name = (char*) malloc(sizeof(char) * 2); 

  if(persona == NULL){
    printf("Error: can't initialize the struct individuo");
    exit(EXIT_FAILURE);
  }
  
  persona->tipo[0] = 'B';
  persona->tipo[1] = '\0';
  
  persona->name[0] = 'B';
  persona->name[1] = '\0';
  persona->genoma = 200;
     
  return persona;
}

int eliminaindividuo(pid_t pid){
  int j, l, i = 0;
  int flag = 0;
  
  while(i < init_people && flag == 0){
    if(persone[i]->pid == pid){
      nome = (char*) malloc(sizeof(char) * strlens(persone[i]->name) + 1);
      for(l = 0; l < strlens(persone[i]->name); l++){
        nome[l] = persone[i]->name[l];
      }  
      nome[strlens(persone[i]->name) + 1] = '\0';
      gen = persone[i]->genoma;
    
      for(j = i; j < init_people - 1; j++){
        persone[j] = persone[j + 1];
      }
      
      persone[init_people - 1] = NULL;
      flag = 1;
    }
  }
  
  return flag; 
}

/**
 *  legge il file con i valori presdefiniti per l'esecuzione
 */
void leggi_file(){
  FILE *fp = fopen("config.txt", "r");
  char *line_buffer = (char *) malloc(sizeof(char) * MAX_LENGHT);
  size_t buffer_size = MAX_LENGHT;
  int data[DATA];
  int i;
  
  if(fp == NULL){
    printf("Error: can't find the file config.txt\n");
    exit(EXIT_FAILURE);
  }
  
  if(line_buffer == NULL) {
    printf("Error allocating memory for line buffer.\n");
    exit(1);
  }
  
  getline(&line_buffer, &buffer_size, fp);
  
  for(i = 0; i < DATA; i++){
    data[i] = atoi(strsep(&line_buffer, ";"));
  }

  init_people = data[0];
  genes = data[1];
  birth_death = data[2];
  sim_time = data[3];
  
  fclose(fp);
}


/**
 *  Crea i due semafori che saranno usati da tutti i processi
 */
/*int crea_semaforo(){
  int sem_id = createsem(KEY_SEM);
  
  if(sem_id == -1){
    printf("Errore nella creazione del set di semafori\n");
    exit(EXIT_FAILURE);
  }
  
  setsemval(sem_id, 0, 1);
  
  return sem_id;
}*/


/**
 *  rimuove l'individuo più vecchio contenuto nell'array persone
 */
void rimuovi_individuo(individuo* persone){
  tmp_individuo = (individuo) malloc(sizeof(individuo));
  int i;
  
  tmp_individuo = persone[0];
  for(i = 0; i < init_people - 1; i++){
    persone[i] = persone[i + 1];
  }
}


/**
 *  crea il processo A o B 
 */
void crea_persona(individuo p){
  char* buf = (char*) malloc(sizeof(unsigned long));
  buf = unslong_to_string(p->genoma);
				
	if(strcmp(p->tipo, "A\0") == 0){				  
	  execlp("./processo_A", p->name, buf,NULL);
	}
	else
	  execlp("./processo_B", p->name, buf, NULL);
}


/**
 *  confronta il genoma dell'individuo rimosso 
 */
void uccidi_individuo(individuo *persone){
  rimuovi_individuo(persone);
    
  kill(tmp_individuo->pid, SIGTERM);
  //signal(SIGQUIT, handler_sigterm);
    
  //pulisci_persona(tmp_individuo);
  //free(tmp_individuo->name);
  //free(tmp_individuo);
}


/**
 *  aumenta la variabile popolazione in base a quale individuo è stato creato
 */
void aumenta_popolazione(individuo p){
  if(strcmp(p->tipo, "A\0") == 0){
    popolazione[0] += 1;
  }else{
    popolazione[1] += 1;
  }
}

int valuta_info(){
  pid_t pid = msgp_r.m.pid;

  printf("Sono stato contattato dal processo con pid: %lu\n", pid);
  return 1;
}



int main() {
  //char *line_buffer = (char *) malloc(sizeof(char) * MAX_LENGHT);
  int i, status, elind1, elind2;
  pid_t child_pid, wpid;
  void *puntatore_shm;
  //char persona_pointer[sizeof(individuo)];
  int sem_id;
  int msgq_id;
  //clock_t inizio, bdclock;
  time_t inizio, fine;
  double esecuzione = 0;
  int cont = 1;
  char* nome1;
  char* nome2;
  unsigned long gen1, gen2;
  popolazione_attuale = init_people;
  pid_t pid1, pid2;
  
  srand(time(NULL));
  
  leggi_file();

  persone = (individuo*) malloc(sizeof(individuo) * init_people);
  //persone = (individuo*) malloc(sizeof(individuo) * 2);
  
  puntatore_shm = createshm(KEY_MEMORIA, sizeof(_individuo) * init_people, &shm_id);
  
  if(puntatore_shm == NULL){
    printf("Errore nella creazione della memoria condivisa\n");
    exit(EXIT_FAILURE);
  }
  
  sem_id = semget(KEY_SEM, 1, IPC_CREAT | IPC_EXCL | 0666);
  if(sem_id == -1){
    printf("Errore nella creazione dei semafori\n");
    exit(EXIT_FAILURE);
  }
  if(initSemInUse(sem_id, 0) == -1){
    printf("Errore nell'inizializzazione del semaforo: %s\n", strerror(errno));
  }
  /*if(getsemval(sem_id, 0) == -1){
    printf("Errore nell'inizializzazione del semaforo: %s\n", strerror(errno));
  }*/
  
  
  msgq_id = msgget(KEY_MSGQ, IPC_CREAT | IPC_EXCL | 0666);
  if(msgq_id == -1){
    printf("Errore nella creazione della coda di messaggi\n");
    exit(EXIT_FAILURE);
  }
  
  /*persone[0] = crea_individuoA();
  persone[1] = crea_individuoB();*/
  
  for(i = 0; i < init_people; i++){
    persone[i] = init_individuo(NULL, NULL, 2, genes);
    aumenta_popolazione(persone[i]);
  //for(i = 0; i < 2; i++){
    switch(child_pid = fork()){
			case -1:
			  perror("errore fork()");
				exit(EXIT_FAILURE);
				break;
				
			case 0: // caso del figlio
				crea_persona(persone[i]);
				break;
				
			default: // caso del padre
			  persone[i]->pid = child_pid;
			  //while ((wpid = wait(&status)) > 0);
				break;
		}
  }
  
  
  sleep(1.0);
  if(releaseSem(sem_id, 0) != 0){
    printf("Errore nell'inizializzazione del semaforo: %s\n", strerror(errno));
    fflush(stderr);
  }
  
  /*inizio = clock();
  bdclock = inizio;*/
  inizio = time(NULL);

  while(esecuzione < sim_time){
    while(msgrcv(msgq_id, &msgp_r, sizeof(struct msgbuf) - sizeof(long), getpid(), IPC_NOWAIT) > 0){
      pid1 = msgp_r.m.data; // pid del processo che invia il messaggio
      pid2 = msgp_r.m.pid;  // pid del processo che è stato scelto per accoppiarsi
      
      elind1 = eliminaindividuo(pid1);
      if(elind1 == 1){
        nome1 = (char *) malloc(sizeof(char) * strlens(nome) + 1);
        gen1 = gen;
        
        for(i = 0; i < strlens(nome); i++){
          nome1[i] = nome[i];
        }  
        nome1[strlens(nome) + 1] = '\0';
        popolazione_attuale--;
        printf("GESTORE: eliminato individuo\n");
      }
      
      elind2 = eliminaindividuo(pid2);
      if(elind2 == 1){
        nome2 = (char *) malloc(sizeof(char) * strlens(nome) + 1);
        gen2 = gen;
        
        for(i = 0; i < strlens(nome); i++){
          nome2[i] = nome[i];
        }  
        nome2[strlens(nome) + 1] = '\0';
        popolazione_attuale--;
        printf("GESTORE: eliminato individuo\n");
      }      
      
      if(elind1 + elind2 == 2){
        persone[popolazione_attuale] = init_individuo(nome1, nome2, MCD(gen1, gen2), genes);
        popolazione_attuale++;
        aumenta_popolazione(persone[i]);
        printf("GESTORE: creato nuovo individuo\n");
        
        persone[popolazione_attuale] = init_individuo(nome1, nome2, MCD(gen1, gen2), genes);
        popolazione_attuale++;
        aumenta_popolazione(persone[i]);
        printf("GESTORE: creato nuovo individuo\n");
      }else if(elind1 + elind2 == 1){
        printf("Errore nell'eliminazione di uno dei genitori: %lu - %lu\n", pid1, pid2);
      }
    }
    
    if((esecuzione / cont) > birth_death){
      printf("Uccido l'individuo più vecchio\n");   
      reserveSem(sem_id, 0);
      uccidi_individuo(persone);
      persone[init_people - 1] = init_individuo(NULL, NULL, 2, genes);
      crea_persona(persone[init_people - 1]);
      aumenta_popolazione(persone[init_people-1]);
      releaseSem(sem_id, 0);
      cont++;
    }
    fine = time(NULL); 
    esecuzione = difftime(fine, inizio); 
  }

  /*while(1){
    if(msgrcv(msgq_id, &msgp_r, sizeof(struct msgbuf) - sizeof(long), getpid(), 0) > 0){
      valuta_info();
      i++;
    }  
  }*/
  while ((wpid = wait(&status)) > 0);
  printf("%d processi A creati.\n", popolazione[0]);
  printf("%d processi B creati.\n", popolazione[1]);
  
  //print_shm(puntatore_shm, init_people);

  /*for(i = 0; i <init_people; i++){
    //pulisci_persona(persone[i]);
  } */
  //print_shm(puntatore_shm, init_people);
}

