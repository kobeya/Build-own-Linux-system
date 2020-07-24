#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

/*1, exit():exit函数是在_exit函数之上的一个封装，其会调用_exit，并在调用之前会刷新.
 *
 *2, _exit()关闭进程所有的文件描述符，清理内存以及其他一些内核清理函数，
 * 但不会刷新流（stdin,stdout,stderr…)
 *
 *CLR:https://www.cnblogs.com/gangzilife/p/9088275.html
 *
 *
 * 简单总结： 
 * 首先，在退出前，我们调用了一些高级的I/O函数，这里是printf,这些函数在操作的时候，
 * 数据都会通过一个I/O缓存区来加速文件的读写。
 * 这个缓存区有个特点：在等待满足一定的条件才会把缓存区的内容一次性写入文件
 * 这个条件包括：达到一定的数量，或者遇到特定字符，如换行和文件结束符EOF
 *
 *举个例子：printf("hello, world");//这个就没有达到写入文件条件，	
 *所以，内存都保存在这个缓存区中。
 * 此时若是使用exit()退出，则先把缓存区的内容先写入文件，也就是先打印内容，才会退出。
 * 但是若是使用_exit()退出，则缓存区的内容直接丢弃，直接退出。
 *
 * */

int main(void)
{
	pid_t pid,wait_pid;
	int status;

	pid = fork();

	if (pid==-1)	{
		perror("Cannot create new process");
		exit(1);
	} else 	if (pid==0) {
		printf("child process id: %ld\n", (long) getpid());
		pause();
		_exit(0);
	} else {
		printf("parent process id: %ld\n", (long) getpid());
		wait_pid=waitpid(pid, &status, WUNTRACED | WCONTINUED);
		if (wait_pid == -1) {
			perror("cannot using waitpid function");
			exit(1);
		}

		if(WIFSIGNALED(status))
			printf("child process is killed by signal %d\n", WTERMSIG(status));

		exit(0);
	}
}
