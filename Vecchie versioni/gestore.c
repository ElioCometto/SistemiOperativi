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
#include "library.h"

#define DATA 4
#define MAX_LENGHT 20
#define strlens(s) (s==NULL?0:strlen(s))


/*
*
* All’inizio, il gestore crea un numero init_people ≥ 2
* (per esempio 20) di individui. Per ogni individuo creato, vengono determinati
* casualmente:
* • il suo tipo: “A” o “B”;
* • il suo nome: un carattere maiuscolo (da “A” a “Z”);
* • il suo genoma: un unsigned long casuale da 2 a 2+genes.
* La creazione degli individui avviene con fork e poi una execve del figlio.
* Non appena creati, i processi “individuo” devono attendere la creazione di
* tutti gli altri, prima di iniziare il proprio ciclo di vita.
* Quindi, il gestore aspetta la terminazione dei propri figli e, come vedremo,
* genera nuovi individui una volta che altri sono terminati. Difatti, la termina-
* zione di una coppia di processi di tipo diverso determina la creazione di una
* nuova coppia di processi con caratteristiche “ereditate” dai due processi che si
* sono accoppiati e che sono poi terminati.
* Ogni birth_death secondi il gestore:
* 1. termina un processo con un segnale (attenzione: la terminazione istanta-
* nea del processo selezionato con SIGTERM potrebbe determinare uno stato
* inconsistente delle strutture dati. Esempio: cosa succede se viene ucciso
* un processo “B” la cui proposta viene accettata “troppo tardi”? Cosa
* succede se viene ucciso un processo “A” appena dopo che un processo
* “B” lo ha contattato?). La scelta del processo da terminare è a scelta del
* progettista. Di seguito alcuni esempi:
* • casuale;
* • il processo più vecchio;
* • il processo con genoma più piccolo;
* • il processo con il nome più breve;
* • altro;
* 2. crea un nuovo processo con le stesse modalità della creazione iniziale e lo
* immette nella popolazione;
* 3. aggiorna l’utente sullo stato della simulazione.
* Dopo sim_time dalla creazione iniziale, la simulazione termina. Il gestore
* informa tutti i processi presenti che devono terminare e fa in modo che non
* rimangano in stato zombie. Il gestore rilascia tutte le risorse eventualmente
* utilizzate. Dopo la terminazione, il processo gestore stampa alcune statistiche
* della simulazione che includono:
* • il numero di processi di ciascun tipo vissuti durante tutta la simulazione;
* • tutte le caratteristiche del processo con il nome più lungo (in seguito si
* vedrà che il nome di un processo potrà anche essere più lungo di un solo
* carattere);
* • tutte le caratteristiche del processo con il genoma (che è un intero) più
* grande;
* • altre informazioni ritenute utili.
* Le quantità init_people, genes, birth_death e sim_time sono lette dal-
* l’utente a run time (da stdin, da un file testo di configurazione, da riga di
* comando, da variabili di ambiente, etc. a scelta).
*
*/


/*  Crea un individuo */
individuo init_individuo(char* nome_padre, char* nome_madre, int x, int gene){
  char* tipo = (char *) malloc(sizeof(char));
	char* nome = (char *) malloc(sizeof(char) * (strlens(nome_padre) + strlens(nome_madre) + 2));
	char nome_tmp;
	unsigned long* genoma = (unsigned long*) malloc(sizeof(unsigned long));
  individuo persona = (individuo) malloc(sizeof(individuo));
  int i = 0, j = 0;
  

  if(tipo == NULL || nome == NULL || genoma == NULL || persona == NULL){
    printf("Error: can't initialize the struct individuo");
    exit(EXIT_FAILURE);
  }
  
  if(rand() % 2){
    *tipo = 'A';
  }else{
    *tipo = 'B';
  }
  
  nome_tmp = 65 + (rand() % 25);
  if(nome_padre != NULL || nome_madre != NULL){
    for(i = 0; i < strlen(nome_padre); i++){
      nome[i] = nome_padre[i];
    }
    for(j = 0; j < strlen(nome_madre); j++){
      nome[i+j] = nome_madre[j];
    }
    nome[i + j] = nome_tmp;
    nome[i + j + 1] = '\0';
  }else{
    nome[0] = nome_tmp;
    nome[1] = '\0';
  }
  printf("\n");
   
  *genoma = x + (rand() % (x + gene));
  
  persona->tipo = *tipo;
  strcpy(persona->name, nome);
  persona->genoma = *genoma;
  
  free((void *) tipo);
  free((void *) nome);
  free((void *) genoma);
     
  return persona;
}


/*  Esegue la stampa  di tutti gli attributi di un individuo  */
void debug_individuo(individuo test){
  printf("Genere : %c\n", test->tipo);
  printf("Nome: %s\n", test->name);
  printf("Genoma: %lu\n", test->genoma);
}


int main() {
  FILE *fp = fopen("config.txt", "r");
  char *line_buffer = (char *) malloc(sizeof(char) * MAX_LENGHT);
  int data[DATA];
  int init_people, genes, birth_death, sim_time, status;
  int i;
  size_t buffer_size = MAX_LENGHT;
  pid_t child_pid, wpid;
  individuo persona;
  
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
  
  for(i = 0; i < init_people; i++){
    persona = init_individuo(NULL, NULL, 2, genes);
    
    switch(child_pid = fork()){
			case -1:
			  perror("errore fork()");
				exit(EXIT_FAILURE);
				break;
				
			case 0: // caso del figlio
				debug_individuo(persona);
				
				free((void*) persona);
				exit(1);
				break;
				
			default: // caso del padre
				while ((wpid = wait(&status)) > 0); 
				break;
		}
  }
  printf("Sono il padre. Tutti i figli hanno finito l'esecuzione.\n");
  
}




