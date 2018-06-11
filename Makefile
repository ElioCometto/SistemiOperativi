program: gestore.o processo_A.o processo_B.o sharedmem.o semaphore.o
	gcc -o program gestore.o sharedmem.o semaphore.o
	gcc -o processo_A processo_A.o sharedmem.o semaphore.o
	gcc -o processo_B processo_B.o sharedmem.o semaphore.o
		
gestore.o: gestore.c sharedmem.h semaphore.h
	gcc -c gestore.c
	
processo_A.o: processo_A.c sharedmem.h semaphore.h
	gcc -c processo_A.c

processo_B.o: processo_B.c sharedmem.h semaphore.h
	gcc -c processo_B.c
	
sharedmem.o: sharedmem.c sharedmem.h
	gcc -c sharedmem.c
	
semaphore.o: semaphore.c semaphore.h
	gcc -c semaphore.c

