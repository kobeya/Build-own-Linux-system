#if 0 
本文主要从三个方面去分析讨论COW机制

1，fork的时候，内核为COW做了哪些准备？
2，COW进程是如何触发？
3，内核如何处理COW这种异常？
4，匿名页的reuse


一、fork一个子进程的时候，内核做了哪些准备？
	在fork内部，_do_fork -> copy_process -> copy_mm
	在复制父进程页表的死后，内核会判断这个页表条目是完全复制，还是修改当前为只读-> COW
	简单理解，就是通过一些判断，来协助将父子进程的页表项属性改为只读，请注意，
	这个时候的vma是可读写的，共享相同的物理页，明显这里的共享，指的是当前进程的页表项
	里面表示的映射关系是一样的，也就是共享内容相同的页表，然后这份页表指向同一块物理
	内存。


二、COW缺页异常触发条件

	1，如果父子进程也仅仅是只读共享内存，那么此刻就会通过各自的页表，然后读取共享的物理内存里面
		的数据，啥事也不会异常。

	2，如果父子任何一个进程要写，那么就会给处理器发送一个异常信号，代码里面有一个判断：

		if(vmf > flags & FAULT_FLAG_WRITE) { //这里如果，VMA可写，那么就继续往下判断
			if(!pte_write(entry)) //页表项只读，
				return do_wp_page(vmf);//触发COW
		}

	简单来说，就是当一个进程想要写的时候，会做执行两个判断：
		1）当前进程的mm_struct里面的成员vma属性可写
		2）当前进程的这个页表项属性却只是可读
		那么逻辑就触发缺页异常，并进入处理函数

三、内核怎么处理COW这种异常？

	1，首先为写的一方分配一个新的物理页
	2，然后将之前共享物理页里面的内存，拷贝到新的物理页
	3，然后更新页表，建立新的物理页与虚拟页的映射关系
	4，最后返回到产生缺页异常的那条指令重新执行。



#endif


#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int tmp = 100;

int main(int argc, char* argv[])
{

	pid_t pid;
	
	printf("%s\t%d pid=%d\ttmp=%d\n", __func__, __LINE__, getpid(), tmp);

	pid = fork();

	if(pid < 0) {
		printf("fork error!\n");
		return -1;
	}

	if(pid == 0) {
	/*这里发生了非常多的事情，按照目前章节主题，可以描述如下：
	 *	1，对只读属性的页表项对应的物理页写
	 *	2，检查发现mm_struct里面的VMA单元是可写的
	 *	3，触发缺页异常，进入缺页异常处理函数
	 *	4，找到新的物理页
	 *	5，把原来页表映射的物理页的内存，拷贝到新的物理页上
	 *	6，修改页表项
	 *	7，返回重新执行引起缺页异常的指令
	 *	8，对tmp赋值
	 * */
		tmp = 120;
		printf("### This is child process pid=%d  num=%d###\n", getpid(), tmp);
		_exit(0);
	}
	if (pid > 0) {
		sleep(1);
		printf("### This is parent process pid=%d  num=%d###\n", getpid(), tmp);
		_exit(0);
	}


	return 0;
}
