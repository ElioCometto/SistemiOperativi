OBJECTS = gestore.o processo_A.o processo_B.o sharedmem.o semaphore.o
CC = gcc
EXEC = program processo_A processo_B

$(EXEC): $(OBJECTS)
	$(CC) -o $@ $^

%.o : %.c
	$(CC) -c -o $@ $<
  
clean:
	rm $(OBJECTS) $(EXEC)
  
sharedmem.o: sharedmem.h
gestore.o: sharedmem.h
