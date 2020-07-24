#ifndef __test_H_
#define __test_H_
#include <stdio.h>
#include <pthread.h>
#include <stdio.h>
#include <linux/unistd.h>
#include <sys/syscall.h>


int data = 5;


static void dosomething(void)
{
	printf("child pid = %d\tdata = %d\n", getpid(), data = 20);
	exit(0);
}

int main(int argc, char *argv[])
{
	pid_t pid;
	pid = fork();
	if(pid == -1)
	{
		printf("can't fork a new process!\n");
		exit(1);
	}
	else if(pid == 0)
	{
		dosomething();
	}
	else
	{
		printf("parent ppid = %d\tdata = %d\n", getpid(), data);
	}
	printf("data = %d\n", data);
}





#endif // #ifndef __test_H_

