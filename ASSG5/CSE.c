#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <fcntl.h>
#include <sys/wait.h>

#define BUFF_LEN 512
char *program = "./cse";

int executer(int * std_fd,int * fd1,int *fd2,int i);
int cinput(int *std_fd,int *fd1,int *fd2,int i);




void supervisor(){
	//printf("supervisor");
	int fd1[2];
	int fd2[2];
	
	pipe(fd1);
	pipe(fd2);
	
	printf("hello from supervisor %d %d\n",fd1[0],fd1[1]);
	
	char fd1_0[10],fd1_1[10],fd2_0[10],fd2_1[10];
	sprintf(fd1_0, "%d", fd1[0]); 
	sprintf(fd1_1, "%d", fd1[1]);
	sprintf(fd2_0, "%d", fd2[0]); 
	sprintf(fd2_1, "%d", fd2[1]);  
	
	//char *program = "./cse";
	
	for(int i = 0 ;i < 2; i++){
		pid_t p = fork();
		if(p == 0 && i == 0){
			//commandline
			execlp("xterm", "xterm", "-T", "Command Input", "-e", program, "c",fd1_0,fd1_1,fd2_0,fd2_1, (char *)NULL);
			perror("execlp failed");

			exit(0);
		}
		if(p == 0 && i == 1){
			//execute
			execlp("xterm", "xterm", "-T", "Execution", "-e", program, "e",fd1_0,fd1_1,fd2_0,fd2_1, (char *)NULL);
			perror("execlp failed");
			
			//printf("Im %d my dad is %d, im executer \n",getpid(),getppid());
			exit(0);
		}
	}
	
	//producer
	//printf("Im %d my dad is %d , im producer\n",getpid(),getppid());
	
	
	close(fd1[0]);
    	close(fd1[1]);
    	
    	close(fd2[0]);
    	close(fd2[1]);
    	
    	
	wait(NULL);
	wait(NULL);
	
	
}

int cinput(int *std_fd,int *fd1,int *fd2,int i){
	
	dup2(std_fd[0], STDIN_FILENO);
	dup2(std_fd[1],STDOUT_FILENO);
	
	
	if(i == 0){
		fprintf(stderr,"input suing pipe1\n");
		close(fd1[0]); // Close read end, commandline only writes to pipe1
		dup2(fd1[1], STDOUT_FILENO); // Redirect stdout to write to the pipe1
		//close(fd1[1]);
	}else{
		fprintf(stderr,"input suing pipe2\n");
		close(fd2[0]); // Close read end, commandline only writes to pipe2
		dup2(fd2[1], STDOUT_FILENO); // Redirect stdout to write to the pipe2
		//close(fd2[1]);
		
	}
	
	
	char buffer[BUFF_LEN];
	
	//printf("hello");
	//memset(buffer, 0, sizeof(buffer));
	while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
		fprintf(stderr,"Enter Command:");
		buffer[strcspn(buffer,"\n")] = '\0';
		if (strcmp(buffer, "exit") == 0) {
			memset(buffer, 0, sizeof(buffer));
			printf("exit\n", buffer);
			return 0;
			break;
		}
		if (strcmp(buffer, "swaprole") == 0) {
			fflush(stdout);
			printf("swaprole\n");
			fflush(stdout);
			
			return 1;		//returned on request of swap role
		}
        	printf("%s\n", buffer);
        	memset(buffer, 0, sizeof(buffer));
	}
	return 0;
}

int executer(int * std_fd,int * fd1,int *fd2,int i){
	dup2(std_fd[0], STDIN_FILENO);
	dup2(std_fd[1],STDOUT_FILENO);
	
	
	
	
	if(i==0){ 
		fprintf(stderr,"executer suing pipe1\n");
		close(fd1[1]); // Close write end, executer only reads from pipe
		dup2(fd1[0], STDIN_FILENO); // Redirect stdin to read from the pipe
		//close(fd1[0]);
	}
	else{
		fprintf(stderr,"executer suing pipe2\n");
		close(fd2[1]); // Close write end, executer only reads from pipe
		dup2(fd2[0], STDIN_FILENO); // Redirect stdin to read from the pipe
		//close(fd2[0]);
	}
	
	int k = 0;
	char buffer[BUFF_LEN];
	while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
		printf("%d\n",k++);
	
		
		if (strcmp(buffer, "swaprole\n") == 0) {
			
			printf("Catched buffer %s",buffer);
			memset(buffer, 0, sizeof(buffer));
			fflush(stdout);
			
			
			return 1;
			break;
		}
		
		if (strcmp(buffer, "exit\n") == 0) {
			
			
			memset(buffer, 0, sizeof(buffer));
			fflush(stdout);
			
			
			return 0;
			break;
		}
		
		if(fork() == 0){
			dup2(std_fd[0], STDIN_FILENO);
			int ret = system(buffer);
			if (ret == -1) {
			    perror("system");
			    return -1;
			}
			exit(0);
		}
		else{
			wait(NULL);
		}
	}
	return 0;
}


int main(int argc, char ** argv){
	int saved_fds[2];
	saved_fds[0] = dup(STDIN_FILENO);
	saved_fds[1] = dup(STDOUT_FILENO);
	
	if(argc == 1){
		supervisor();
	}
	else if(argv[1][0] == 'e'){
		//printf("executer");
		int fd1[2];
		int fd2[2];
		fd1[0] = atoi(argv[2]); // Convert FD string back to int
		fd1[1] = atoi(argv[3]);
		fd2[0] = atoi(argv[4]); // Convert FD string back to int
		fd2[1] = atoi(argv[5]);
		
		
		
		int r = 1;
		int i = 0;
		while(r != 0){
			if(r == 1){
				if(i == 0){
					//executer on executer window
					fprintf(stderr,"exec ran on exec window\n");
					r = executer(saved_fds,fd1,fd2,i);
					i = 1;
				}
				else{
					//input on executer window
					fprintf(stderr,"input ran on exec window\n");
					r = cinput(saved_fds,fd1,fd2,i);
					i = 0;
				}
			}
		}
	}
	else if(argv[1][0] == 'c'){
		
		int fd1[2];
		int fd2[2];
		fd1[0] = atoi(argv[2]); // Convert FD string back to int
		fd1[1] = atoi(argv[3]);
		fd2[0] = atoi(argv[4]); // Convert FD string back to int		
		fd2[1] = atoi(argv[5]);
		printf("hello from command %d %d\n",fd1[0],fd1[1]);
		
		/*
		close(fd1[0]); // Close read end, commandline only writes to pipe
		dup2(fd1[1], STDOUT_FILENO); // Redirect stdout to write to the pipe
		close(fd1[1]);
		*/
		//printf("hello");
		int r = 1;
		int i = 0;
		while(r != 0){
			if(r == 1){
				fprintf(stderr,"swap requested\n");
				if(i == 0){
					fprintf(stderr,"input ran on input window\n");
					r = cinput(saved_fds,fd1,fd2,i);
					i = 1;
				}
				else{
					fprintf(stderr,"execute ran on input window\n");
					r = executer(saved_fds,fd1,fd2,i);
					//r = cinput(saved_fds,fd1,fd2,i);
					i = 0;
				}
			}
		}
	}
	
}
