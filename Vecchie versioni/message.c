#include <stdio.h>
#include "library.h"
#include "sharedmem.h"
#include "semaphore.h"

float target;
individuo me;

static void handler(int sig){
	if (sig == SIGTERM){
		libera_risorse();
		kill(getpid,0);
	}
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

void scrivi_info(individuo me, unsigned long *meshm){
  int i = 0, flag = 0;
  getsemval(sem_id);
  locksem(sem_id,0);

  while(i<5 && flag == 0){
  	if(meshm[i].pid == 0){
  		strcpy(meshm[i].tipo, me->tipo);
  		meshm[i].genoma = me->genoma;
  		meshm[i].pid = getpid();
  		flag = 1
  	}

  	i++;
  }
  unlocksem(sem_id,0);
}

void ricevi_messaggio(pid_t pid){
 //Leggi dalla coda(processo in wait)
  if(msgrcv(msgq_id, &msgp,sizeof(individuo), pid,0) == -1){
    printf("Errore nel ricevere info di B\n");
    exit(EXIT_FAILURE);
  }
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

void valuta_info(void* msgp){
	unsigned long genB = strtoul(msgp->mtext, NULL, 10); //ci siamo salvati il genoma del processo B
	unsigned long mcd = MCD(genB, me->genoma);
	if(mcd == me->genoma){
		return 1;
	}
	else {
		if(mcd >= target){
			return 1;
		} 
		else return -1;
	}
}

void abbassa_target(){
	target = target * 0.90;
}

void invia_messaggio(int stato, pid_t pid_destinatario, pid_t pid_oggetto){
	struct msgbuf sbuf;

	sbuf->mtype = pid_destinatario;
	sbuf->mtext[0] = stato; // stato di accetatto (1), rifiutato(0)
	sbuf->mtext[1] = pid_oggetto;

	  //Scrivi su coda di messaggi OK(verso processo B)
    if(msgsnd(msgq_id, &sbuf, sizeof(sbuf),0)){
      printf("Errore nel rispondere al processo B\n");
      exit(EXIT_FAILURE);
    }
}

void libera_risorse(){
	if(msgctl(msgq_id, IPC_RMID, NULL) == -1){
      printf("Errore nella chiusura della coda di messaggi\n");
      exit(EXIT_FAILURE);
    }
    if(shmdt(sharedmem) == -1){
      printf("Errore nel staccarsi dalla memoria condivisa\n");
      exit(EXIT_FAILURE);
    }
    pulisci_persona(me);
}

int main(int argc, char* argv[]) {
  me = initialize_individuo(argv[0], strtoul(argv[1], NULL, 10));
  int shm_id, i;
  void *shm;
  int sem_id, msgq_id;
  target = me->genoma;
  int va_bene;
  void *msgp;

  sem_id = semget(KEY_SEM, 0, 0);
  getsemval(sem_id);
  
  printf("Sono il processo_A\n");
  debug_individuo(me);
  
  shm_id = shmget(shm_key, 0, 0);
  shm = (unsigned long *) shmat(shm_id, NULL, 0);
  if(shm == NULL){
    printf("Errore: memoria condivisa NON trovata\n");
    exit(EXIT_FAILURE);
  }
  
  scrivi_info(me, shm);
  
  //print_shm(shm);
  
  //Crea la coda dei messaggi 
  msgq_id = msgget(KEY_MSGQ,0);

  while(1){
    msgrcv(msgq_id,msgp, sizeof(unsigned long)*2, me->pid, 0);
    if(msgq_id == -1) {
      printf("Errore: coda dei messaggi NON trovata\n");
      exit(EXIT_FAILURE);
    } 

    //Valuta informazioni di B 
    va_bene = valuta_info(msgp);
    
    if (va_bene){
    	//Invia al processo B il messaggio e gli comunica che è stato accetatto
    	invia_messaggio(1, pidB, me->pid);

    //Invia al gestore il pid del processo B con cui si è accoppiato
      invia_messaggio(getppid(),pidB);

    //Termina liberando risorse 
      libera_risorse();
    } else {
    //Scrivi su coda di messaggi NO(verso processo B)
      invia_messaggio(0, pidB, me->pid);
      abbassa_target();
       
    }
    sleep(0.5);
  }
}
