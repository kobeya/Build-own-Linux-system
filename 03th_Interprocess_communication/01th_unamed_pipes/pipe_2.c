/*
 *	(GNU v3.0) kobey 2020年08月09日13:52:42
 *	用户空间无法实现，进程间通讯。
 *	借助无名管道的例子
 *	Q，为什么称呼为无名？
 *	A，是指这个管道在文件系统中不存在文件名，没有文件节点，通过打开的文件描述符来实现队列。
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
 *	A，pipe创建管道之后，父子进程体内，都包含两个在创建管道时打开的文件描述符。
 *	并且父子进程中的两个文件描述符，指向同一个无名管道缓冲区，也就是，父子进程，都可以
 *	往管道缓冲区中写，也可以读，只是注意一点，不管是读，还是写的当前状态，会先关闭另外一端，
 *	比如，父进程在写fd[1]，那么就会先停止fd[0]的读。
 *	同样，可以引申到，子进程的子进程，同样可以读取父进程的写，不过，记住，只能读一次！
 *	
 *	Q，读没有数据的管道会发生什么情况，写满的管道又会发生什么情况
 *	A，都会自我阻塞。
 *
 *	Q，无名管道其实也是文件，那么这个文件在内存中是什么的表现形态？
 *	A，在系统调用层次中，它是一个用16个管道缓冲区组成的一页内存。
 *
 *
 * */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

	pid_t pid;
	int process_inter = 0;
	int ret1;
	int fd[4];
	ret1 = pipe(fd);
	if(ret1 < 0) {
		return -1;
	}

	pid = fork();

	if(pid < 0)
		printf("create process failure\n");
	if(pid == 0)
	{
		int sun;
		read(fd[0], &process_inter, 1);
		/*这里做一个阻塞，当process_inter恒等于0，就跑到死循环中阻塞
		 *如果process_inter收到父进程的修改，不恒等于0，就不会跑到死循环
		 *继续执行下面的打印语句。
		 * */
		while(process_inter == 0);
		printf(" sun read process_inter = %d\n", process_inter);
		for(int i = 0; i < 5; i++) {
			printf("this is child process i = %d\n", i);
		}
	//	close(fd[0];
		process_inter = 2;
		sun = write(fd[1], &process_inter, 1);	
		if(sun < 0)
		{
			printf("sun write failture!\n");
		}else {
			printf("sun write sucess, sun = %d\n", sun);
		}
		if(sun == 0){
			printf("2\n");
		}
		read(fd[0], &process_inter, 1);
		while(process_inter == 1);
		sleep(3);
		sun = read(fd[0],&process_inter, 1);
		printf("666\n");
//		while(process_inter == 3);
		if(sun < 0) {
			printf("second read error\n");
		}
		process_inter = 5;
		write(fd[1], &process_inter, 1);
		printf("1\n");

		sleep(8);
		printf("8\n");
		pid_t pid1;
//		read(fd[0], &process_inter, 1);
		printf("end,process_inter = %d\n", process_inter);
		pid1 = fork();
		printf("pid1 = %d\n", pid1);
		if(pid1 == 0)
		{
			read(fd[0], &process_inter, 1);
			printf("end,process_inter = %d\n", process_inter);
		}
		sleep(1);
		read(fd[0], &process_inter, 1);
		printf("end_sun,process_inter = %d\n", process_inter);
	}
	if(pid > 0) {
		for(int i = 0; i < 5; i++) {
			printf("this is parent process i = %d\n", i);
		}
		process_inter = 4;
//		sleep(1);
		write(fd[1], &process_inter, 1);
		printf("parent write done\n");
		sleep(1);
		process_inter =3;
		write(fd[1], &process_inter, 1);
		sleep(5);
		read(fd[0], &process_inter, 1);
		if(process_inter == 5) {
			printf("888\n");
		}
		process_inter = 6;
		write(fd[1], &process_inter, 1);
	#if 0
//		close(fd[1]);
		read(fd[0], &process_inter, 1);
		if(process_inter == 2)
		{
			printf("parent read done!\n");
		}
		else{
			printf("parent read error!\n");
		}
	#endif
	}
	while(1);
	return 0;
}
