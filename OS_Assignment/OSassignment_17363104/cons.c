//The Consumers

#include <math.h>            //for log()
#include <unistd.h>          //for sleep()
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>       //for sem 
#include <fcntl.h>           //for O_flags
#include<sys/mman.h>         //for PROT_WRITE MAP_SHARED
#include <pthread.h>
#define BUFFER_SIZE 20
typedef int buffer_item;

sem_t *full;
sem_t *empty;
sem_t *lock;
void *ptr;
double param_c; 
pthread_t tid[3];

struct buf{
    int num;
    buffer_item data[BUFFER_SIZE];
};

void *consumer(void *);
double negative_exp(double);

int main(int argc, char*argv[]){		                 //check input argument
	if(argc==1){
		printf("No input parameter.\n");
		return -1;
	}
	else if (argc >2){
		printf("Invalid input.\n");
		return -1;
	}
	
	int shm_fd = shm_open("buffer",O_RDWR,0666);           //open shared memory
	ptr = mmap(0,sizeof(struct buf),PROT_READ | PROT_WRITE,MAP_SHARED,shm_fd,0);  

	pid_t id= getpid();                                    //print process id
	printf("Consumers' Pid is: %d\n",id);
	pthread_attr_t attr[3];	
	
	param_c = atof(argv[1]);
	
	full = sem_open("full",O_CREAT);
	empty = sem_open("empty",O_CREAT);
	lock = sem_open("lock",O_CREAT);
	
	int t_no[3];
			
	for(int i=0;i<3;i++){                                //join thread
		pthread_attr_init(&(attr[i]));
		t_no[i] = i;
		pthread_create(&(tid[i]), &(attr[i]), consumer, &(t_no[i]));	
	}
	for(int i=0;i<3;i++){
		pthread_join(tid[i],NULL);	
	}
	
	return 0;
}

void *consumer(void *param){                           
    int id = *(int *)param;
    while (1)
	{ 
		int sleep_time=(int)negative_exp(param_c);
        sleep(sleep_time);

       	sem_wait(full);          
        sem_wait(lock);	
        struct buf *buf_ptr = ((struct buf *)ptr);
		buf_ptr->num = (buf_ptr->num-1) % BUFFER_SIZE;
        buffer_item data = buf_ptr->data[buf_ptr->num];                                  //get the data and consume it                
		printf("Consumed data[%d]: %d, by thread[%d], tid: %ld\n",buf_ptr->num,data,id,tid[id]);
		sem_post(lock);
        sem_post(empty);
    }
    pthread_exit(0);
}
double negative_exp(double d){
    double n=1.0;
    while((n==0) || (n == 1))
    {
        n = ((double)rand() / RAND_MAX);          //produce a number between 0 and 1
    }
   
    return (-1.0/d * log(n)*600);
}

