/*
 *	(GNU v3.0) kobey 2020年08月09日13:52:42
 *	用户空间无法实现，进程间通讯的例子。
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
