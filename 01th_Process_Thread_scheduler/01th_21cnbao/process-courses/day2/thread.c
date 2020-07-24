#include <stdio.h>
#include <pthread.h>
#include <stdio.h>
#include <linux/unistd.h>
#include <sys/syscall.h>

/*背景：Linux内核中并没有实现线程，而是由glibc线程库实现的POSIX线程。
 * 每个线程也有一个id，类型 pthread_t(unsigned long int)，由pthread_self()取得，
 * 该id由线程库维护，其id空间是各个进程独立的（即不同进程中的线程可能有相同的id）。
 *1，通过对应的系统调用号，返回线程的tig
 *2, getpid,返回进程pid，实际是gtid,包括我们看到top中的pid，也就是这个gtid，这里设计
 * 一个小技巧，就是Linux设计成这样，让你看上多线程的进程呈现出一个唯一的pid。我们知道，
 * 在调度中，每个task_struct对应一个唯一的pid，线程也拥有task_struct,所以，它们也拥有
 * 独立的pid。
 * */


/* 1，使用syscall实现一个gettid函数，用于获取线程的pid，或者称为tig
 * 2，在2.6.19之前，使用_syscall汇编嵌入的宏，但是从2.6.19开始，废除了，使用
 * syscall，通过指定系统调用号，和一组参数来调用系统调用。
 *
 * */

static pid_t gettid( void )
{
	//return syscall(SYS_gettid);这样写也可以，因为现在的系统调用，几乎都包括SYS_符号常量来映射自己的系统调用号。
	return syscall(__NR_gettid);
}

static void *thread_fun(void *param)
{
	printf("thread_fun:\tthread pid:%d, tid:%d pthread_self:%lu\n", getpid(), gettid(),pthread_self());
	while(1);
	return NULL;
}

int main(void)
{
	pthread_t tid1, tid2;
	int ret;

	printf("thread pid:%d, tid:%d pthread_self:%lu\n", getpid(), gettid(),pthread_self());
	/*在man手册中，我们可以查看pthread_create在做些什么事情
	 * 1，创建tid1线程，
	 * 2，这个新的线程，在thread_fun中开始运行
	 * */
	ret = pthread_create(&tid1, NULL, thread_fun, NULL);
	if (ret == -1) {
		perror("cannot create new thread");
		return -1;
	}

	ret = pthread_create(&tid2, NULL, thread_fun, NULL);
	if (ret == -1) {
		perror("cannot create new thread");
		return -1;
	}

	if (pthread_join(tid1, NULL) != 0) {
		perror("call pthread_join function fail");
		return -1;
	}

	if (pthread_join(tid2, NULL) != 0) {
		perror("call pthread_join function fail");
		return -1;
	}

	return 0;
}
