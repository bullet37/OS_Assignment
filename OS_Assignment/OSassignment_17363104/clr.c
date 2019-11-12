#include <unistd.h>          //for sleep()
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>       //for sem 
#include <fcntl.h>           //for O_flags
#include<sys/mman.h>         //for PROT_WRITE MAP_SHARED

sem_t *full;
sem_t *empty;
sem_t *lock;
int main(int argc, char*argv[]){
	full = sem_open("full",O_CREAT);
	empty = sem_open("empty",O_CREAT);
	lock = sem_open("lock",O_CREAT);
	sem_close(full);
	sem_unlink("full");
	sem_close(empty);
	sem_unlink("empty");
	sem_close(lock);
	sem_unlink("lock");
	shm_unlink("buffer");
	return 0;
}
