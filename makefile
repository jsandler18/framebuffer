CC=gcc
CFLAGS= -c -Wall -O3
INCLUDE=include
OBJECTS=obj
VPATH=fb:linalg:%.h $(INCLUDE):%.o $(OBJECTS)

fb.x: fb.o draw.o linalg.o
	$(CC) $^ -o $@

$(OBJECTS)/fb.o: fb.c 
	$(CC) $(CFLAGS) -I$(INCLUDE) $< -o $@

$(OBJECTS)/draw.o: draw.c draw.h
	$(CC) $(CFLAGS) -I$(INCLUDE) $< -o $@

$(OBJECTS)/linalg.o: linalg.c linalg.h
	$(CC) $(CFLAGS) -I$(INCLUDE) $< -o $@

clean:
	rm -f obj/*
	rm -f fb.x
