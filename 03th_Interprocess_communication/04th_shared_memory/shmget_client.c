/*
 *	(GNU v3.0) kobey 2020年08月11日11:19:38
 *	进程间通讯5——共享内存实现无亲缘关系进程间通讯
 *
 *	框架、原理：
 *		1，两个进程通过ftok生成的相同的key去创建相同的共享内存索引
 *		2，server进程 | client进程分别映射到自己的用户空间
 *		3，server进程先往共享内存中写入自己的pid
 *		4，server自我睡眠，等待client进程读取server的pid
 *		5，client读取了server的pid，再写入自己的pid
 *		6，client通过获取到的server pid，发送信号唤醒
 *		7，server读取client写入的pid
 *		8，server client此刻都拥有了对方的pid，并且拥有相同的共享内存，实现通讯。
 *					   key=ftok("*", '*');
 *					|			|
 *	模型：		进程server-->口				进程client-->口
 *			send:server.pid||get clien.pid		|send:client.pid;get server.pid
 *		————————————————————————————————————————————————————————————————
 *					口口口口通过一样的key，创建一样的shmid，也就是同一个共享内存
 *		内核空间		
 *
 *		————————————————————————————————————————————————————————————————
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

struct shm_buf {
	int pid;
	char buf[156];
};

int main()
{
	int shmid;
	struct shm_buf *p =NULL;
	int key;
	int pid = -1;
	key = ftok("./key", 'y');
	if(key < 0) {
		printf("server ftok key failure\n");
		return -1;
	}
	shmid = shmget(key, 160,IPC_CREAT | 0777);
	if(shmid < 0) {
		printf("create shmget failure\n");
		return -2;
	}
	printf("server shmget share memory sucess\n");
	p = (struct shm_buf *)shmat(shmid, NULL, 0);
	if(p == NULL) {
		printf("server shmat failure\n");
		return -3;
	}
	signal(SIGUSR2, sig_function);
	pid = p->pid;
	if(pid < 0) {
		printf("client read server.pid failure\n");
		return -4;
	}
	p->pid = getpid();
	kill(pid, SIGUSR1);
	while(1)
	{
		pause();
		printf("client read share memory data = %s\n", p->buf);
		kill(pid, SIGUSR1);
	}



	shmdt(p);
	shmctl(shmid, IPC_RMID, NULL);
	//man手册可以看到，system读取字符串，fork一个shell窗口执行字符串表示的指令
	system("ipcs -m");
	return 0;
}

