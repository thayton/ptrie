CC = gcc

CFLAGS = -Wall -g

testpatricia: testpatricia.o patricia.o
	$(CC) -o testpatricia testpatricia.o patricia.o

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<
clean:
	rm -f testpatricia testpatricia.o patricia.o