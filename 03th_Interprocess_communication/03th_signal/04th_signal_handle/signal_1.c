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
#include <sys/types.h>
#include <sys/wait.h>

void myfuc(int signum)
{
	printf("signal = %d,myfuc\n", signum);
}

void myfuc1(int signum)
{
	printf("signal = %d,myfuc1\n", signum);
	/*	如果没有直接包含定义api的头文件，也许其他头文件有重复包含，但是编译器可能还是会
	 *	报出一些警告信息，例如：
	 *	warning: implicit declaration of function ‘wait’; 
	 *	did you mean ‘write’? [-Wimplicit-function-declaration]
	 *	wait(NULL);
	 *	
	 *	消除这些警告的方法很简单，man api,把头文件包含进来就可以。
	 * */
	wait(NULL);//如果父进程没有wait子进程，子进程退出后就变成僵尸进程了
	return;
}

int main(int argc, char *argv[])
{
	pid_t pid;

	pid = fork();
	if(pid < 0) {
		printf("fork error\n");
		return -1;
	}
	if(pid > 0) {
		int i = 0;
		signal(10, myfuc);
		signal(17, myfuc1);
		while(i < 10) {
			printf("parent %d\n", i);
			sleep(1);
			i++;
		}
		while(1);
	}
	if(pid == 0) {
		sleep(5);
		kill(getppid(), 10);


		exit(0);//可以查看一下API的目录，可以看到这个包含了向父进程发送17信号。	
	}
	return 0;
}
