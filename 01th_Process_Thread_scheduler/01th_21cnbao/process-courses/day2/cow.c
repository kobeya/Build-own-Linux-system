#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int data = 10;

int child_process()
{
	printf("Child process %d, data %d\n",getpid(),data);
	/*全局变量data这里发生非常耗时的变化：
	 *1，首先，这个data在没有执行之前，与父进程共享地址空间，也就是
	 *这个data的执行，指向父进程同样的data拷贝，此时，data的权限是只读的
	 *2，对data赋值之前，首先出现缺页异常，需要重新映射，也就是通过mmu
	 *重新把虚拟地址映射到新的物理地址，返回并让PC指针再次指向原来的指令地址
	 *3，再行运行赋值data = 20。
	 * */
	data = 20;
	printf("Child process %d, data %d\n",getpid(),data);
	_exit(0);//正常退出子进程并且不打印，保留i/o信息。
}

int main(int argc, char* argv[])
{
	int pid;
	pid = fork();

	if(pid==0) {
		child_process();
	}
	else{
		sleep(1);
		printf("Parent process %d, data %d\n",getpid(), data);
		exit(0);
	}
}
