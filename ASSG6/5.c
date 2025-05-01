#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#define ATM_COUNT 5


int atmId[ATM_COUNT];
int balance = 10000;

pthread_mutex_t lock;

void * atm(void * arg){
	int * ID = arg;
	int noOfOp = (rand() % 15) + 1;
	//int noOfOp = 10000;
	for(int i = 0 ; i < noOfOp ; i++){
		int op = (rand() % 100) < 50;
		int amount = (rand() % 1000) + 1;
		if(!op){
			//deposit
			pthread_mutex_lock(&lock);
				balance = balance + amount;
				printf("ATM %d deposited: %d | Balance: %d\n",*ID,amount,balance);
			pthread_mutex_unlock(&lock);
			sleep(1);
		}
		else{
			//withdrawal
			pthread_mutex_lock(&lock);
				if(balance >= amount){
					balance = balance - amount;
					printf("ATM %d Withdrawn: %d | Balance: %d\n",*ID,amount,balance);

				}
				else{
					printf("ATM %d. Insufficient balance! Withdrawal req:%d | Balance: %d\n",*ID,amount,balance);
				}
			pthread_mutex_unlock(&lock);
			sleep(1);
		}
	}
}

int main(){
	srand(getpid());
	pthread_t atm_t[ATM_COUNT];
	pthread_mutex_init(&lock,NULL);
	
	for(int i = 0 ; i < ATM_COUNT; i++){
		atmId[i] = i;
	}
	
	for(int i = 0; i < ATM_COUNT ; i++){
		pthread_create(&atm_t[i],NULL,&atm,&atmId[i]);
	}
	
	
	
	for(int i = 0; i < ATM_COUNT ; i++){
		pthread_join(atm_t[i],NULL);
	}
	
	pthread_mutex_destroy(&lock);

}
