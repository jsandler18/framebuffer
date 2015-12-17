CC=gcc
CFLAGS= -c -Wall -O3
INCLUDE=include
VPATH=fb:linalg:$(INCLUDE)

fb.x: fb.o draw.o linalg.o
	$(CC) $< fb.o $< draw.o $< linalg.o -o fb.x

fb.o: fb.c 
	$(CC) $(CFLAGS) -I$(INCLUDE) $< -o $@

draw.o: draw.c draw.h
	$(CC) $(CFLAGS) -I$(INCLUDE) $< -o $@

linalg.o: linalg.c linalg.h
	$(CC) $(CFLAGS) -I$(INCLUDE) $< -o $@

