#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#define PHIL_COUNT 5

int left(int i){
	return (i - 1 + PHIL_COUNT) % PHIL_COUNT;
}

int right(int i){
	return (i + 1) % PHIL_COUNT;
}

int state[PHIL_COUNT];		//0 = thinking, 1 = hungry, 2 = eating
int phil_i[PHIL_COUNT];


sem_t cr_mutex;
sem_t out_mutex;
sem_t bothForkAvail[PHIL_COUNT];

void test(int i){	//if hungry and both neighbours are not eating, then eat
	if(state[i] == 1 && state[left(i)] != 2 && state[right(i)] != 2){
		state[i] = 2;			//is eating
		sem_post(&bothForkAvail[i]);	//release the forks
	}
}

void think(int i){
	sem_wait(&out_mutex);
		printf("%d is thinking\n",i);
		fflush(stdout);
	sem_post(&out_mutex);
	sleep(1);
}

void take_forks(int i){
	sem_wait(&cr_mutex);
		state[i] = 1;		//is hungry
		sem_wait(&out_mutex);
			printf("%d is hungry\n",i);
			fflush(stdout);
		sem_post(&out_mutex);
		test(i);	//trying to acquire a permit(trying to free some fork to use here)
	sem_post(&cr_mutex);
	
	sem_wait(&bothForkAvail[i]);
}

void eat(int i){
	sem_wait(&out_mutex);
		printf("%d is eating\n",i);
		fflush(stdout);
	sem_post(&out_mutex);
	sleep(1);
}

void put_forks(int i){
	sem_wait(&cr_mutex);
		state[i] = 0;		//is thinking
		test(left(i)); 		//trying to acquire a permit(trying to free some fork to use here) for left
		test(right(i)); 	//trying to acquire a permit(trying to free some fork to use here) for left
	sem_post(&cr_mutex);
}

void * philosopher(void * num){
	while(1){
		int * i = num;
		think(*i);
		take_forks(*i);
		eat(*i);
		put_forks(*i);
	}
}

int main(){
	for(int i = 0 ; i < PHIL_COUNT ; i++){
		phil_i[i] = i;
	}
	sem_init(&cr_mutex,0,1);
	sem_init(&out_mutex,0,1);
	
	for(int i = 0 ; i < PHIL_COUNT ; i++){
		sem_init(&bothForkAvail[i],0,0);
	}
	
	pthread_t philosophers_t[PHIL_COUNT];
	
	for(int i = 0 ; i < PHIL_COUNT ; i++){
		pthread_create(&philosophers_t[i],NULL,philosopher,&phil_i[i]);
	}
	for(int i = 0 ; i < PHIL_COUNT ; i++){
		pthread_join(philosophers_t[i],NULL);
	}
	
}
