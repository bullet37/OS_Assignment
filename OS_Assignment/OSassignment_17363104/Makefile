CC=gcc
CFLAGS=-Wall
RT=-lrt
POSIXT=-lpthread
MATH=-lm 
all:
	$(CC) $(CFLAGS) dph.c -o dph $(RT) $(POSIXT)
	$(CC) $(CFLAGS) prod.c -o prod $(RT) $(POSIXT) $(MATH)
	$(CC) $(CFLAGS) cons.c -o cons $(RT) $(POSIXT) $(MATH)	
	$(CC) $(CFLAGS) mycall.c -o mycall

dph:
	$(CC) $(CFLAGS) dph.c -o dph $(RT) $(POSIXT)

prod:
	$(CC) $(CFLAGS) prod.c -o prod $(RT) $(POSIXT) $(MATH)

cons:
	$(CC) $(CFLAGS) cons.c -o cons $(RT) $(POSIXT) $(MATH)

mycall:
	$(CC) $(CFLAGS) mycall.c -o mycall
    
clean:
	rm -rf dph
	rm -rf prod
	rm -rf cons
	rm -rf mycall
