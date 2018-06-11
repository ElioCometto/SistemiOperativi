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

/**  
 *  Crea un individuo 
 */
individuo init_individuo(char* nome_padre, char* nome_madre, int x, int gene){
	char nome[strlens(nome_padre) + strlens(nome_madre) + 2];
	char nome_tmp;
	unsigned long genoma;
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
int crea_semaforo(){
  int sem_id = createsem(KEY_SEM);
  
  if(sem_id == -1){
    printf("Errore nella creazione del set di semafori\n");
    exit(EXIT_FAILURE);
  }
  
  setsemval(sem_id, 0, 1);
  
  return sem_id;
}


/**
 *  rimuove l'individuo più vecchio contenuto nell'array persone
 */
individuo rimuovi_individuo(individuo* persone){
  individuo tmp = (individuo) malloc(sizeof(individuo));
  int i;
  
  tmp = persone[0];
  for(i = 0; i < init_people - 1; i++){
    persone[i] = persone[i + 1];
  }
  
  return tmp;
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
  individuo tmp_individuo = rimuovi_individuo(persone);
  int i = 0;
    
  kill(tmp_individuo->pid, SIGTERM);
    
  pulisci_persona(tmp_individuo);
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



int main() {
  char *line_buffer = (char *) malloc(sizeof(char) * MAX_LENGHT);
  int i, status;
  pid_t child_pid, wpid;
  individuo* persone;
  void *puntatore_shm;
  char persona_pointer[sizeof(individuo)];
  int sem_id;
  clock_t inizio, bdclock;
  
  srand(time(NULL));

  leggi_file();
  printf("file letto\n");
  persone = (individuo*) malloc(sizeof(individuo) * init_people);
  
  //puntatore_shm = createshm(shm_key, (sizeof(unsigned long) * 3) * init_people, &shm_id);
  puntatore_shm = createshm(shm_key, sizeof(_individuo) * init_people, &shm_id);
  printf("shm creata\n");
  printf("%d\n", (void*)puntatore_shm);
  if(puntatore_shm == NULL){
    printf("Errore nella creazione della memoria condivisa\n");
    exit(EXIT_FAILURE);
  }
  sem_id = crea_semaforo();
  printf("semaforo creato\n");
  for(i = 0; i < init_people; i++){
    persone[i] = init_individuo(NULL, NULL, 2, genes);
    aumenta_popolazione(persone[i]);
   
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
			  while ((wpid = wait(&status)) > 0);
				break;
		}
  }
  printf("fork e execve\n");
  unlocksem(sem_id, 0);
  /*inizio = clock();
  bdclock = inizio;*/
  
 /* 
  while(((clock() - inizio) / CLOCKS_PER_SEC) < sim_time){
    printf("Tempo di esecuzione: %lu\n", (clock() - inizio) / CLOCKS_PER_SEC);
    if(((clock() - bdclock) / CLOCKS_PER_SEC) > birth_death){   
      locksem(sem_id, 1);
      uccidi_individuo(persone);
      persone[init_people - 1] = init_individuo(NULL, NULL, 2, genes);
      crea_persona(persone[init_people - 1]);
      aumenta_popolazione(persone[init_people-1]);
      unlocksem(sem_id, 1);
    }
    bdclock = clock();
  }
  */ 
  
  while ((wpid = wait(&status)) > 0);
  printf("%d processi A creati.\n", popolazione[0]);
  printf("%d processi B creati.\n", popolazione[1]);
  
  print_shm(puntatore_shm, init_people);

  for(i = 0; i <init_people; i++){
    //pulisci_persona(persone[i]);
  } 
  //print_shm(puntatore_shm, init_people);
}

