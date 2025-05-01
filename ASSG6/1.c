#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>


int shared = 5;
sem_t mutexSem;

void * fun1(void * arg){
	printf("Value of shared before fun1: %d\n",shared);
	fflush(stdout);
	
	sem_wait(&mutexSem);
	//critical
	int currVal = shared;
	currVal++;
	sleep(1);
	shared = currVal;
	//end critical
	sem_post(&mutexSem);
	
	printf("Value of shared after fun1: %d\n",shared);
	fflush(stdout);
}

void * fun2(void * arg){
	
	printf("Value of shared before fun2: %d\n",shared);
	fflush(stdout);
	sem_wait(&mutexSem);
	//critical
	int currVal = shared;
	currVal--;
	sleep(1);
	shared = currVal;
	//end critical
	sem_post(&mutexSem);
	printf("Value of shared after fun2: %d\n",shared);
	fflush(stdout);
}

int main(){
	sem_init(&mutexSem,0,1);
	pthread_t t1,t2;
	pthread_create(&t1,NULL,fun1,NULL);
	pthread_create(&t2,NULL,fun2,NULL);
	pthread_join(t1,NULL);
	pthread_join(t2,NULL);
	sem_destroy(&mutexSem);
}
