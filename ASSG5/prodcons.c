#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <wait.h>
#include <stdlib.h>
#include <time.h>

int main() {
	int n,t;
	scanf("%d %d",&n,&t);
	
	
	int *M = mmap(0, sizeof(int) * 2, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	M[0] = 0;
	
	printf("The producer works here\n");
	
	srand(time(NULL));
	
	pid_t f_return;
	for(int i = 0 ; i < n ; i++){
		f_return = fork();
		if(f_return == 0){
			//our consumer code
			printf("\t\t\t\tConsumer %d is alive\n",i+1);
			int count =0;
			int sum = 0;
			while(1){
				int item = 0;
				while(M[0] != i+1 && M[0] != -1);
				if(M[0] == -1){
					break;
				}
				
				item=M[1];
				count++;
				printf("\t\t\t\titem of %d = %d\n",M[0],M[1]);
				sum=sum+item;
				
				
				M[0] = 0;
				
				
			}
			printf("\t\t\t\tConsumer %d has read %d items: Checksum = %d\n",i+1,count,sum);
			exit(0);
			
			
		}
	}
	
	
	int count[n];
	int sum[n];
	for(int i = 0; i < n ; i ++){
		count[i]=0;
		sum[i]=0;
	}
	
	for(int i = 0 ; i < t ; i++){
		int val = rand() % 900 + 100;
		int c = rand() % n;
		//printf("val = %d , c = %d\n",val,c+1);
		while(M[0] != 0);
		//printf("heelo%d\n",i);
		
		M[0]=c+1;
		#ifdef SLEEP
		usleep(10);
		#endif
		
		M[1]=val;
		
		count[c]++;
		sum[c]=sum[c]+val;
		
		
		#ifdef VERBOSE
		printf("Producer produces %d for Consumer %d\n",M[1],M[0]);
		#endif
	}
	
	while(M[0] != 0);
	M[0] = -1;
	
	
	for(int i=0;i<n;i++){ // loop will run n times (n=5) 
    		wait(NULL); 
	}
	
	printf("Producer has produced %d items\n",t);
	for(int i=0;i<n;i++){ // 
    		printf("%d items for Consumer %d: Checksum = %d\n",count[i],i+1,sum[i]);
	}
	munmap(M, sizeof(int) * 2);
	
	return 0;
}

