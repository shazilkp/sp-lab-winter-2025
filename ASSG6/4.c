#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

sem_t doneSmoking;

sem_t tobaccoAndPaper;
sem_t paperAndMatches;
sem_t matchesAndTobacco;

int smokerId[3];

//1 =tob pap ,2 = pap match,3 = match tob

void * agent(void * arg){
	while(1){
		int item1 = rand() % 3 + 1;
		int item2 = rand() % 3 + 1;
		int val = item1 * item2;
		//release corresponding semaphore
		sleep(2);
		switch(val){
			case 1:{		//tob pap
				sem_post(&tobaccoAndPaper);
				printf("\nAgent placed tob and pap\n");
				fflush(stdout);
				break;
			}
			
			case 2:{		//pap match
				sem_post(&paperAndMatches);
				printf("\nAgent placed pap and matches\n");
				fflush(stdout);
				break;
			}
			
			case 3:{		//match tob
				sem_post(&matchesAndTobacco);
				printf("\nAgent placed matches and tob\n");
				fflush(stdout);
				break;
			}
		}
	}
}

void * smoker(void * num){
	while(1){
	int *i = num;
	switch(*i){
		case 0:{
			sem_wait(&tobaccoAndPaper);		//wait on tob,pap
				printf("Smoker %d acquired tob and pap\n",(*i)+1);
				fflush(stdout);
				
				sem_wait(&doneSmoking);		//acquire done smoking
					sleep(1);		//wait
					printf("Smoker %d smoked\n",(*i)+1);
					fflush(stdout);
				sem_post(&doneSmoking);	//release done smokinh
			//sem_post(&tobaccoAndPaper);
			break;
		}
		case 1:{
			sem_wait(&paperAndMatches);		//wait on pap,matches
				printf("Smoker %d acquired pap and matches\n",(*i)+1);
				fflush(stdout);
				
				sem_wait(&doneSmoking);		//acquire done smoking
					sleep(1);		//wait
					printf("Smoker %d smoked\n",(*i)+1);
					fflush(stdout);
				sem_post(&doneSmoking);	//release done smokinh
			//sem_post(&paperAndMatches);
			break;
		}
		case 2:{
			sem_wait(&matchesAndTobacco);		//wait on matches,tob
				printf("Smoker %d acquired matches and tob\n",(*i)+1);
				fflush(stdout);
				
				sem_wait(&doneSmoking);		//acquire done smoking
					sleep(1);		//wait
					printf("Smoker %d smoked\n",(*i)+1);
					fflush(stdout);
				sem_post(&doneSmoking);	//release done smokinh
			//sem_post(&matchesAndTobacco);
			break;
		}
	}
	}
	
}

int main(){
	srand(getpid());
	for(int i = 0 ; i < 3 ; i++){
		smokerId[i] = i;
	}
	
	
	sem_init(&doneSmoking,0,1);
	
	sem_init(&tobaccoAndPaper,0,0);
	sem_init(&paperAndMatches,0,0);
	sem_init(&matchesAndTobacco,0,0);
	
	pthread_t agent_t;
	pthread_t smoker_t[3];
	
	pthread_create(&agent_t,NULL,agent,NULL);
	for(int i = 0 ; i < 3 ; i++){
		pthread_create(&smoker_t[i],NULL,smoker,&smokerId[i]);	
	}
	
	pthread_join(agent_t,NULL);
	for(int i = 0 ; i < 3 ; i++){	
		pthread_join(smoker_t[i],NULL);
	}
	
	
	
	
}
