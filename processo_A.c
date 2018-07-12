#include <stdio.h>
#include <errno.h>
#include "library.h"
#include "sharedmem.h"
#include "semaphore.h"

float target;
individuo me;
int sem_id, msgq_id;
struct msgbuf msgp;
void *shm;
unsigned long pidB;


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
  //pers->name[i + 1] = '\0';
  pers->name[strlens(name)] = '\0';
  pers->genoma = gen;
  
  pers->pid = getpid();
  
  return pers;
}

void scrivi_info(individuo meshm){
  int i = 0, flag = 0;

  reserveSem(sem_id, 0);

  while(i < 5 && flag == 0){
  	if(meshm[i].pid == 0){
  		strcpy(meshm[i].tipo, me->tipo);
  		meshm[i].genoma = me->genoma;
  		meshm[i].pid = getpid();
  		flag = 1;
  	}

  	i++;
  }
  releaseSem(sem_id, 0);
}


unsigned long MCD(unsigned long gen1, unsigned long gen2){
	unsigned long r; //resto
	unsigned long a = gen1;
	unsigned long b = gen2; 

	 while(b != 0){
	 	r = a % b;
	 	a = b;
	 	b = r;
	 }
	 
	 return a;
}

int valuta_info(){
  //char *msg = ((struct msgbuf)msgp)->mtext;
	//unsigned long genB = strtoul(msg, NULL, 10); //ci siamo salvati il genoma del processo B
	unsigned long genB = msgp.m.data;
  pidB = msgp.m.pid;
	unsigned long mcd = MCD(genB, me->genoma);
	
	printf("Sono stato contattato dal processo B con pid: %lu\n", genB);
	
	if(mcd == me->genoma){
		return 1;
	}
	else {
		if(mcd >= target){
			return 1;
		} 
		else 
		  return -1;
	}
}

void abbassa_target(){
	target = target * 0.90;
}

void invia_messaggio(unsigned long stato, pid_t pid_destinatario, pid_t pid_oggetto){
	struct msgbuf sbuf;

	sbuf.mtype = pid_destinatario;
	sbuf.m.data = stato; // stato: accetatto (1), rifiutato(2)
	sbuf.m.pid = pid_oggetto;

	  //Scrivi su coda di messaggi OK(verso processo B)
    if(msgsnd(msgq_id, &sbuf, sizeof(sbuf) + 1, 0)){
    printf("Errore nello scrivere sulla coda di messaggi: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
}

void libera_risorse(){
	if(msgctl(msgq_id, IPC_RMID, NULL) == -1){
      printf("Errore nella chiusura della coda di messaggi\n");
      exit(EXIT_FAILURE);
    }
    if(shmdt(shm) == -1){
      printf("Errore nel staccarsi dalla memoria condivisa\n");
      exit(EXIT_FAILURE);
    }
    pulisci_persona(me);
    exit(1);
}


void (*handler(int sig)){
	if (sig == SIGTERM){
		libera_risorse();
	}
}

int main(int argc, char* argv[]) {
  me = initialize_individuo(argv[0], strtoul(argv[1], NULL, 10));
  int shm_id, i;
  target = me->genoma;
  int va_bene;
  
  printf("Sono il processo_A\n");
  sem_id = semget(KEY_SEM, 0, 0666);
  
  while(getVal(sem_id) != 1){
    sleep(0.5);
  }
  
  reserveSem(sem_id, 0);
  debug_individuo(me);
  fflush(stdout);
  releaseSem(sem_id, 0);
  
  shm_id = shmget(KEY_MEMORIA, 0, 0);
  shm = (individuo) shmat(shm_id, NULL, 0);
  if(shm == NULL){
    printf("Errore: memoria condivisa NON trovata\n");
    fflush(stderr);
    exit(EXIT_FAILURE);
  }
  
  scrivi_info(shm);
  
  //print_shm(shm, 5);
  
  //Crea la coda dei messaggi 
  msgq_id = msgget(KEY_MSGQ, 0);
  if(msgq_id == -1) {
    printf("Errore: coda dei messaggi NON trovata\n");
    fflush(stderr);
    exit(EXIT_FAILURE);
  } 
  
  while(1){
    //if(msgrcv(msgq_id, &msgp, sizeof(msgp.m), me->pid, 0) > 0){
    if(msgrcv(msgq_id, &msgp, sizeof(struct msgbuf) + 1, me->pid, 0) > 0){
      //Valuta informazioni di B 
      va_bene = valuta_info();
      
      if (va_bene){
      	//Invia al processo B il messaggio e gli comunica che è stato accetatto
      	invia_messaggio(1, pidB, me->pid);

        //Invia al gestore il pid del processo B con cui si è accoppiato
        invia_messaggio(getpid(), getppid(), pidB);

        //Termina liberando risorse 
        libera_risorse();
      } else {
      //Scrivi su coda di messaggi NO(verso processo B)
        invia_messaggio(2, pidB, me->pid);
        abbassa_target();
      }       
    }/*else{
      printf("Impossibile ricevere messaggio: %s\n", strerror(errno));
    }*/
    sleep(0.5);
  }
}
