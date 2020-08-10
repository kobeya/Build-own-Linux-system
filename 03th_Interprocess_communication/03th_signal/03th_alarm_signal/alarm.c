/*
 *	(GNU v3.0) kobey
 *	alarm，进程按照设定的时间发送信号给自己，相当于一个信号定时器，接受信号的默认操作：终止。
 *	unsigned int raise(unsigned int seconds);
 *	成功：
 *		一、如果已经设置了定时器，那么返回已经设置的定时器的剩余时间；
 *		二、如果没有设置过定时器，则设置信号发送的定时功能，收到信号后，进程终止，返回0；
 *		三、失败，返回-1。
 *	功能：这个函数本身不会阻塞，它只是在设置的当前，设置进程会在外来某个时刻，终止，
 *	这个定时区间，进程，还是正常执行的。说得通俗一点就是，程序员完全可以通过信号alarm给自己
 *	设置一个定时器，进程执行到多久之后，自行终止！
 * */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int i = 0;
	printf("before alarm\n");
	alarm(5);
	printf("after alarm\n");
	while(i < 10)
	{
		printf("1s\n");
//		sleep(10);//alarm接口设置定时器，该进程不能处于终止状态，否则无法响应。
//		pause(); //处于S 睡眠状态的进程可以响应alarm定时器的信号
		raise(SIGTSTP); //暂停T或者Z状态的进程，无法响应alarm定时器的信号。
	}
	return 0;
}
