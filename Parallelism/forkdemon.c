#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/wait.h>

int main(){
	pid_t pid;
	pid = fork();
	if (pid==0){
		printf("Demon runs...\n");
		sleep(2);
		char* command = malloc(10);
		int val;
		scanf("%s",command);
		scanf("%d",&val);
		printf("command inputted: %s", command);
		return 5;

	}
	else{
		printf("Parent process finished");
		return 0;
	}
}
