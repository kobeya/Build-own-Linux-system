/*
 *	(GNU v3.0) kobey
 *	raise，进程发送信号给自己
 *	raise(int sig);
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	pid_t pid;

	pid = fork();
	if(pid < 0) {
		printf("fork error\n");
		return -1;
	}
	if(pid > 0) {
		sleep(2);
		if(wait(pid, NULL, WNOHANG) == 0) {
			kill(pid, 9);
		}
		wait(NULL);
		while(1);
	}
	if(pid == 0) {
		raise(SIGTSTP);
		exit(0);	
	}
	return 0;
}
