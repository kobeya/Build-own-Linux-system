/*
 *	(GNU v3.0) kobey 2020年08月11日09:42:20
 *	进程间通讯4——共享内存实现父子或者亲缘进程之间的通讯
 *
 *	框架、原理：
 *		1，父进程需要先创建共享内存，然后fork后，与子进程共享shmid。
 *		2，如果fork后才创建共享内存，父子两个进程内部的共享内存就独立了
 *		3，父子进程需要把内核态的共享内存，分别做一次映射到自己的用户空间
 *		4，父进程写，写完发信号给子进程
 *		5，子进程读，读完也可以发信号给父进程
 *	备注：
 *		1，编程需要注意一些细节，比如，信号处理函数，必须在pause睡眠前注册，否则无法响应的。
 *		2，pause这个函数的行为，是阻塞当前进程，让进程进入睡眠态，接受信号的到来，
 *		跳转到signal信号处理函数中，当signal返回后，pause也就返回-1。
 * */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void sig_function(int signum) {
	return;
}

int main()
{
	int shmid;
	pid_t pid;
	char *p = NULL;
	shmid = shmget(IPC_PRIVATE, 160,IPC_CREAT | 0777);
	if(shmid < 0) {
		printf("create shmget failure\n");
		return -1;
	}
	pid = fork();
	if(pid < 0) {
		printf("fork failure\n");
		exit(1);
	}
	if(pid > 0)
	{
		printf("parent active:\n");
		signal(SIGUSR2, sig_function);
	//	char *p = NULL;
		p = (char *)shmat(shmid, NULL, 0);
		if(p == NULL) {
			printf("parent process shmat failure\n");
			return -2;
		}
		sleep(1);
		while(1) {
#if 1
			pause();
			printf("parent read shmat data = %s\n", p);
			kill(pid, SIGUSR1);
#endif
#if 0
			printf("ipput parent fget share memory data:");
			fgets(p, 160, stdin);
			kill(pid, SIGUSR1);//给子进程发信号，唤醒
			pause();//睡眠，等待进程信号的到来，并且等待信号处理函数返回后，pause才返回-1
#endif
		}
	}

	if(pid == 0)
	{
		printf("child process activite:\n");
		signal(SIGUSR1, sig_function);
		printf("register child signale!\n");
	//	char *p =NULL;
		p = (char *)shmat(shmid, NULL, 0);
		if(p == NULL) {
			printf("child process shmat failure\n");
			return -3;
		}
		while(1) {
#if 1 //测试子进程写
			fgets(p, 160, stdin);
			kill(getppid(), SIGUSR2);
			pause();
#endif
#if 0
			pause();//等待父进程的信号到来
			printf("child read share memory data = %s\n", p);
			kill(getppid(), SIGUSR2);
#endif
		}
	}
	shmdt(p);
	shmctl(shmid, IPC_RMID, NULL);
	//man手册可以看到，system读取字符串，fork一个shell窗口执行字符串表示的指令
	system("ipcs -m");
	return 0;
}

