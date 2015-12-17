CC=gcc
CFLAGS= -c -Wall -O3
INCLUDE=include
OBJECTS=obj
VPATH=fb:linalg:$(INCLUDE):obj

fb.x: fb.o draw.o linalg.o
	$(CC) $< fb.o $< draw.o $< linalg.o -o fb.x

fb.o: fb.c 
	$(CC) $(CFLAGS) -I$(INCLUDE) $< -o $(OBJECTS)/$@

draw.o: draw.c draw.h
	$(CC) $(CFLAGS) -I$(INCLUDE) $< -o $(OBJECTS)/$@

linalg.o: linalg.c linalg.h
	$(CC) $(CFLAGS) -I$(INCLUDE) $< -o $(OBJECTS)/$@

