#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

/* printf b c a c ,why print b first ? because printf very complicated, it need much time to 
 * run, so maybe sun run first ,but printf parent!
 * */

int main(void)
{
//	pid_t pid,wait_pid;
//	int status;
	pid_t pid;

	pid = fork();//get two processes

	if (pid==-1)	{
		perror("Cannot create new process");
		exit(1);
	} else 	if (pid==0) {	//sun run here
		printf("a\n");
	} else {
		printf("b\n"); // parent run here
	}

	printf("c\n"); // parent and sun run here together
	while(1);
	return 0;
}
