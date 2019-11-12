//The producer
// gcc - prod prod.c -lrt -lpthread -lm
#include <unistd.h>          //for sleep()
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>       //for sem 
#include <math.h>
#include <fcntl.h>           //for O_flags
#include<sys/mman.h>         //for PROT_WRITE MAP_SHARED

#define BUFFER_SIZE 20

sem_t *full;
sem_t *empty;
sem_t *lock;                
void *ptr;
pthread_t tid[3];
double param_p; 

void *producer(void *);
double negative_exp(double);


typedef int buffer_item;
struct buf{
	int num;
    buffer_item data[BUFFER_SIZE];    
};



int main(int argc, char*argv[]){
	if(argc==1){                                             //check input argument
		printf("No input parameter.\n");
		return -1;
	}
	else if (argc >2){
		printf("Invalid input.\n");
		return -1;
	}
	
	
	param_p = atof(argv[1]);
	
	pid_t id= getpid();									 //print process id
	printf("Producers' Pid is: %d\n",id);
	
	full=sem_open("full",O_CREAT|O_EXCL,0666,0);        //create semaphoresw
	if(full == SEM_FAILED)
    	{
     	fprintf(stderr,"unable to create the full semaphore");
      	sem_unlink("full");
      	exit(-1);
   	}
	empty=sem_open("empty",O_CREAT|O_EXCL,0666,BUFFER_SIZE);
	if(empty == SEM_FAILED)
    	{
     	fprintf(stderr,"unable to create the empty semaphore");
      	sem_unlink("empty");
      	exit(-1);
   	}

	lock=sem_open("lock",O_CREAT|O_EXCL,0666,1);
	if(lock == SEM_FAILED)
    	{
     	fprintf(stderr,"unable to create the s_mutex semaphore");
      	sem_unlink("lock");
      	exit(-1);
   	}
   	
	int shm_fd = shm_open("buffer",O_CREAT | O_RDWR,0666);                 //Create shared memory
	ftruncate(shm_fd,sizeof(struct buf));
	ptr = mmap(0,sizeof(struct buf),PROT_WRITE|PROT_READ,MAP_SHARED,shm_fd,0);      
	pthread_attr_t attr[3];
	int t_no[3];		
	for(int i=0;i<3;i++){
		pthread_attr_init(&(attr[i]));
		t_no[i] = i;
		pthread_create(&(tid[i]), &(attr[i]), producer, &(t_no[i]));	
	}
	for(int i=0;i<3;i++){
		pthread_join(tid[i],NULL);	
	}
	
	return 0;
}
void *producer(void *param){
	int id = *(int *)param;
    while(1){         
	int sleep_time=(int)(negative_exp(param_p));	
        sleep(sleep_time); 
      
        sem_wait(empty);           //lock                        	
        sem_wait(lock);                                    
                      	  
        buffer_item data = rand() % 100;
        struct buf *buf_ptr = ((struct buf *)ptr);
		(buf_ptr->data)[buf_ptr->num] = data;            
        
		printf("Produce data[%d]: %d, by thread[%d], tid: %ld\n",buf_ptr->num,data,id,tid[id]);
		buf_ptr->num = (buf_ptr->num+1) % BUFFER_SIZE;

        sem_post(lock);                                   //Unlock
        sem_post(full);
	                                   
    }
    pthread_exit(0);
}

double negative_exp(double d){
    double n=1.0;
    while((n==0) || (n == 1))
    {
        n = ((double)rand() / RAND_MAX);        //produce a number between 0 and 1
    }
    return (-1.0/d * log(n)*600);
}






