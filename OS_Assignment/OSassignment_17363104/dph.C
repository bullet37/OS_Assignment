//The Dinning Philosophers problem
#include <unistd.h>          //for sleep()
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum {THINKING,HUNGRY,EATING} state[5];	//Three possible states

void *phi(void *param);
void pick_up_chop(int i);
void put_down_chop(int i);
void func(int signum);
void verify(int i);

//Golbal Varibal
pthread_cond_t cond[5];	 // Behalf on each philosophers
pthread_mutex_t mutex[5]; // For conditional variables
pthread_t tid[5];	// For pthread_create()
pthread_attr_t attr[5];



int main(int argc, char*argv[]){
	int t_no[5];		                 //Philosophers' index
	for(int i=0;i<5;i++){
		pthread_cond_init(&(cond[i]),NULL);
		pthread_attr_init(&(attr[i]));
		t_no[i] = i;
		pthread_create(&(tid[i]), &(attr[i]), phi, &(t_no[i]));	
	}	
	for(int i=0;i<5;i++){
		pthread_join(tid[i],NULL);	
	}
	
}

void *phi(void *param){

    int id = *((int *)param);
    do{
    pick_up_chop(id);                                  //Try to pick up chopsticks
    srand((unsigned)time(NULL));
    int sec = rand()%3 +1;                 
    printf("The NO: %d philosopher eats for %d seconds\n",id+1,sec);  //Eating
    sleep(sec);   
    put_down_chop(id);
    sec = rand()%4 +1; 
    printf("The NO: %d philosopher thinks for %d seconds\n",id+1,sec);  //Thinking
    sleep(sec);
    
    }while(1);
    pthread_exit(NULL);
}

void pick_up_chop(int i){
    state[i] = HUNGRY;                     // Waiting to eat
    verify(i);                               // Check can eat or not
    pthread_mutex_lock(&mutex[i]);
    while (state[i] != EATING){
        pthread_cond_wait(&cond[i],&mutex[i]);//Waiting  his eating neighbors
    }
    pthread_mutex_unlock(&mutex[i]);
}

void put_down_chop(int i){           //Once one philosopher put down chopsticks, 
    state[i] = THINKING;
    verify((i+4)%5);                   //test whether his neighbour need eat.(Producer)
    verify((i+1)%5);
}


void verify(int i){             // Test wheather the NO:i philosopher can eat. Equal to provider
                                   
    if ( (state[(i+4)%5] != EATING)&&(state[i] == HUNGRY) &&(state[(i+1)%5] != EATING)){
        pthread_mutex_lock(&mutex[i]);
        state[i] = EATING;
        pthread_cond_signal(&cond[i]);
        pthread_mutex_unlock(&mutex[i]);
    }

}
