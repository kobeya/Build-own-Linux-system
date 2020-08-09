/*
 *	(GNU v3.0) kobey 2020年08月09日13:52:42
 *	用户空间无法实现，进程间通讯。
 *	借助无名管道的例子
 *	Q，为什么称呼为无名？
 *	A，是指这个管道在文件系统中不存在文件名，通过打开的文件描述符来实现队列。
 *
 *	通讯媒介：内核空间的单向队列。
 *	性质：
 *	1，单向意思是只能一个从一个地方进，另外一个地方出，而且不能改变方向。
 *	2，用队列来缓存临时数据，这个储存的媒介只能临时存放数据，用完自动销毁
 *	3，队列里面的数据，读出来后，里面的数据就没了。
 *	4，当里面的数据被读完，再去读，就会发生阻塞。
 *	5，无名管道也有写阻塞，无名管道有空间限制，5456 5467个单元，
 *		当写满后，再去写，就会写阻塞。
 *	man pipe:
 *	可以看到函数原型：int pipe(int pipefd[2]);
 *	fd[0]是读端口，fd[1]是写端口
 *	成功返回0，失败返回-1。
 *	编程中注意，创建管道后，再不使用需要close来释放空间。
 *
 *	Q，按照一般的理解，在fork之前，创建管道，那么fork一个子进程后，子进程也应该有管道的两个
 *	文件描述符，那么通讯的管道，究竟用的是哪个？
 *
 * */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

	pid_t pid;
	int process_inter = 0;
	
	
	pid = fork();

	if(pid < 0)
		printf("create process failure\n");
	if(pid == 0)
	{
		/*这里做一个阻塞，当process_inter恒等于0，就跑到死循环中阻塞
		 *如果process_inter收到父进程的修改，不恒等于0，就不会跑到死循环
		 *继续执行下面的打印语句。
		 * */
		while(process_inter == 0);
		for(int i = 0; i < 5; i++) {
			printf("this is child process i = %d\n", i);
		}	
	}
	if(pid > 0) {
		process_inter = 10;
		for(int i = 0; i < 5; i++) {
			printf("this is parent process i = %d\n", i);
		}
	}
	while(1);
	return 0;
}
