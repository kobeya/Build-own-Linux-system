/*
 *	(GNU v3.0) kobey 2020年08月10日06:55:49
 *	进程间通讯2——共享内存
 *
 *	Q，前面提到，在创建内核态的共享内存时，key的标志位使用IPC_PRIVATE，如果使用
 *		key = ftok(*, *)来创建共享内存，有什么差别？
 *	A，在使用shmget函数创建共享内存时，使用的这两个不同的key字段，功能相当于无名管道
 *	和有名管道的进程间通讯效果：
 *		IPC_PRIVATE来创建共享内存，只能有亲缘关系的进程间通信
 *		key = ftok()来创建的共享内存，可以实现任意进程间通信。
 *	ftok函数原型：
 *		key_t ftok(const char *pathname, int proj_id);
 *		 |			|		|
 *		 v			v		v
 *	返回的非0整型key值	某个生成key依赖文件	一个字符
 *				文件路径+文件名，	
 * */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main()
{
	int shmid;
	key_t key;
	key = ftok("./key", 'k');
	shmid = shmget(key, 16,IPC_CREAT | 0777);
	if(shmid < 0) {
		printf("create shmget failure\n");
		return -1;
	}

	printf("create shared memroy %d sucess\n", shmid);
	//man手册可以看到，system读取字符串，fork一个shell窗口执行字符串表示的指令
	system("ipcs -m");
	return 0;
}

