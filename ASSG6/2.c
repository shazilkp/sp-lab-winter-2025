#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#define WAREHOUSE_SIZE 10
#define INVENTORY_ITEMS 25

int warehouse[WAREHOUSE_SIZE];
int currSize = 0;

sem_t empty;		//hold the number of empty slots 
sem_t full;		//number of full slots
sem_t mutex;		//binary semaphore


void * producer(void * arg){
	for (int i = 0; i < INVENTORY_ITEMS; i++) {
		//sleep(1);
		sem_wait(&empty); //aquire empty(decrement and see if its > 0) ie ensure there is an empty slot
		
		sem_wait(&mutex);
		//critical section
		
		int currVal = warehouse[currSize];
		currVal = i + 1;
		warehouse[currSize] = currVal;
		currSize++;
		printf("Producer produced: %d (Buffer count: %d)\n",currVal,currSize);
		fflush(stdout);
		
		//end current section
		sem_post(&mutex);
		
		sem_post(&full); //release a full(increment and see if its below limit)
	}
}

void * consumer(void * arg){
	for (int i = 0; i < INVENTORY_ITEMS; i++) {
		sem_wait(&full); //aquire full(decrement and see if its > 0) ie ensure there is an full slot to
		
		sem_wait(&mutex);
		//critical
		
		currSize--;
		int currVal = warehouse[currSize];
		printf("Consumer consumed: %d (Buffer count: %d)\n", currVal, currSize);
		fflush(stdout);
		currVal = -1;
		warehouse[currSize] = currVal;
		
		//endcritical
		sem_post(&mutex);
		
		sem_post(&empty); //release a empty(increment and see if its below limit)
		//sleep(2); // Simulate consumption time
	}
}

int main(){
	sem_init(&mutex,0,1);			//binary semaphore initially unlocked
	sem_init(&empty,0,WAREHOUSE_SIZE);	//initially entire warehouse is empty
	sem_init(&full,0,0);			//nothing is initially full
	
	pthread_t consumer_t,producer_t;
	

	
	pthread_create(&consumer_t,NULL,consumer,NULL);
	pthread_create(&producer_t,NULL,producer,NULL);
	
	pthread_join(consumer_t,NULL);
	pthread_join(producer_t,NULL);
	
	
}
