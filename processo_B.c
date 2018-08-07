#include <stdio.h>
#include <errno.h>
#include "library.h"
#include "sharedmem.h"
#include "semaphore.h"

#define DATA 4
#define MAX_LENGHT 20

int init_people;
int msgq_id;
struct msgbuf msgp;
void *puntatore_shm;
individuo me;
int sem_id;



/* Permette di deallocare la memoria usata dalle malloc della struct _individuo */
void pulisci_persona (){
  free(me->name);
  free(me);
}

void libera_risorse(){
  if(shmdt(puntatore_shm) == -1){
    printf("Errore nel staccarsi dalla memoria condivisa\n");
    exit(EXIT_FAILURE);
  }
  pulisci_persona();
  exit(1);
}

individuo initialize_individuo(char* name, unsigned long gen){
  int i;
  individuo pers = (individuo) malloc(sizeof(individuo));
  pers->name = (char *) malloc(sizeof(char) * (strlens(name) + 1));
  
  if(pers->name == NULL){
    printf("Error: can't initialize the struct in individuo_B");
    exit(EXIT_FAILURE);
  }
  
  pers->tipo[0] = 'B';
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

/*unsigned long MCD(unsigned long genB, unsigned long genA){
	unsigned long r; //resto
	unsigned long a = genB;
	unsigned long b = genA; 

	 while(b != 0){
	 	r = a % b;
	 	a = b;
	 	b = r;
	 }
	 
	 return a;
}*/

int comparator(unsigned long gen1, unsigned long gen2){
  if(MCD(me->genoma, gen1) > MCD(me->genoma, gen2))
    return 1;
  else
    return -1;
}

void invia_messaggio(unsigned long stato, pid_t pid_destinatario, pid_t pid_oggetto){
	struct msgbuf sbuf;
  
	sbuf.mtype = (long) pid_destinatario;
	sbuf.m.data = (unsigned long) stato;
	sbuf.m.pid = pid_oggetto;

  printf("Message type: %lu\n", sbuf.mtype);
  printf("Genoma B: %lu\n", sbuf.m.data);
  printf("Pid B: %d\n\n", sbuf.m.pid);

  if(msgsnd(msgq_id, &sbuf, sizeof(struct msgbuf) - sizeof(long), MSG_NOERROR) != 0){
    if(errno == EACCES){
      printf("EACCESS: %s\n", strerror(errno));
    }
    if(errno == EAGAIN){
      printf("EAGAIN: %s\n", strerror(errno));
    }
    if(errno == EFAULT){
      printf("EFAULT: %s\n", strerror(errno));
    }
    if(errno == EIDRM){
      printf("EIDRM: %s\n", strerror(errno));
    }
    if(errno == EINTR){
      printf("EINTR: %s\n", strerror(errno));
    }
    if(errno == EINVAL){
      printf("EINVAL: %s\n", strerror(errno));
    }
    if(errno == ENOMEM){
      printf("ENOMEM: %s\n", strerror(errno));
    }
    
    //Invalid msqid value, or nonpositive mtype value, or invalid msgsz value  (less
    //than 0 or greater than the system value MSGMAX).

  
    //printf("Errore nello scrivere al processo A: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void ordina_array(individuo *arrA, int numA){
  int i, j;
  individuo tmp;
  int posmax;
  
  for(i = 0; i < numA; i++){
    posmax = i;
    for (j = i + 1; j < numA; j++){
      if(comparator(arrA[j]->genoma, arrA[posmax]->genoma) > 0)
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
  ssize_t nbyte = msgrcv(msgq_id, &msgp, sizeof(struct msgbuf) - sizeof(long), me->pid, 0);
  unsigned long risposta = msgp.m.data;
  pid_t pidA = msgp.m.pid;
  
  if(risposta > 0){
  	printf("Sono stato contattato dal processo A con pid: %lu, risposta: %lu\n", pidA, risposta);
    return risposta;
  }else
    return nbyte;
}

void contatta_processo_A(individuo *arrA, int numA){
  int i = 0;
  int risp, flag = 0;
  
  while(i < numA || flag == 0){
    //printf("Contatto individuo A con pid: %lu\n", arrA[i]->pid);
    invia_messaggio(me->genoma, arrA[i]->pid, me->pid);
    
    sleep(0.25);
    while((risp = leggi_messaggio()) == 0){
      sleep(0.25);
    }
    if(risp > 0){
      if(risp == 1){
        flag = 1; 
        invia_messaggio(getpid(), getppid(), arrA[i]->pid);
        printf("Processo B invio al gestore.\n");
        libera_risorse();
      }else if(risp == 2){
        printf("Sono stato rifiutato dal processo A.\n");
      }
    }
    i++;
  }
  if(flag == 1)
    exit(1);
}

void scegli_A(individuo shm){
  int i, numA;
  individuo *arrA;
  
  arrA = (individuo *) malloc(sizeof(individuo) * init_people);

  numA = 0;
  
  reserveSem(sem_id, 0);
  printf("Scelgo individuo A\n");
  printf("Sezione critica\n");
  fflush(stdout);
  		
  for(i = 0; i < init_people; i++){
    if(shm[i].pid != 0){
      printf("---- Prima di tipo ------\n");
      arrA[numA]->tipo[0] = 'A';
      arrA[numA]->tipo[1] = '\0';
      printf("---- Prima di nome ------\n"); 
      //arrA[numA]->name = (char*) malloc(sizeof(char));
      arrA[numA]->name = NULL;
      printf("---- Prima di genoma -----\n");
      arrA[numA]->genoma = shm[i].genoma;
      arrA[numA]->pid = shm[i].pid;
      debug_individuo(arrA[numA]);
      numA++; 
    }
    fflush(stdout);
  }
    
  releaseSem(sem_id, 0);
  printf("Esco dalla sezione critica\n");
  ordina_array(arrA, numA);
  
  /*for(i = 0; i < numA; i++){
    debug_individuo(arrA[i]);
  }*/
  
  contatta_processo_A(arrA, numA);
  /*for(i = 0; i <init_people; i++){
    if(arrA[i]->genoma != 0){
      pulisci_persona(arrA[i]);
    }
  } */
  //free(arrA);
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

void handler_sigterm(int sig){
  libera_risorse();
}

int main(int argc, char* argv[]) {
  me = initialize_individuo(argv[0], strtoul(argv[1], NULL, 10));
  int shm_id;
  
  printf("Sono il processo_B\n");
  leggi_file();
  sem_id = semget(KEY_SEM, 0, 0666);
  if(sem_id == -1){
     printf("Errore nella letture del semaforo processo B: %s\n", strerror(errno));
  }
  
  while(getVal(sem_id) != 1){
    sleep(0.5);
  }
 
  msgq_id = msgget(KEY_MSGQ, 0);
  if(msgq_id == -1){
     printf("Errore nella lettura della coda di messaggi processo B: %s\n", strerror(errno));
  }
  
  shm_id = shmget(KEY_MEMORIA, 0, 0);
  puntatore_shm = (individuo) shmat(shm_id, NULL, 0);
  if(puntatore_shm == NULL){
    printf("Errore nella lettura della memoria condivisa\n");
    exit(EXIT_FAILURE);
  }
  
  while(1){
    sleep(1.0);
    scegli_A(puntatore_shm);
    fflush(stdout);
  }
}

