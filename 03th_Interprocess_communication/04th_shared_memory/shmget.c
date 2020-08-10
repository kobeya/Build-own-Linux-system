/*
 *	(GNU v3.0) kobey 2020年08月10日06:55:49
 *	进程间通讯1——共享内存
 *	IPC，inter-process communication
 *
 *	IPC，	① 共享内存
 *		② 信号灯semophore
 *		③ 消息队列message queue
 *
 *		这三种IPC通讯模型：
 *
 *
 *
 *
 *	IPC-创建共享内存的API函数原型：
 *	int shmget(key_t key, size_t size, int shmflg);	
 *		key，IPC_PRIVATE或者ftok的返回值
 *		size，共享内存的大小，单位是字节
 *		shmflg,同open函数的权限字段，也可以用8进程来表示
 *		成功返回共享内存的段表示符，文件描述符ID，
 *		失败返回-1。
 *
 *	Q，创建共享内存后，用户空间可以查看对应的信息吗？
 *	A，答：可以，分别有以下API或者工具
 *		ipcs -m，查看当前系统内核已经存在的共享内存及其相关信息
 *		ipcrm -m id，删除IPC共享内存对象命令
 *
 *
 *
 * */

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main()
{
	int shmid;
	shmid = shmget(IPC_PRIVATE, 16, 0777);
	if(shmid < 0) {
		printf("create shmget failure\n");
		return -1;
	}

	printf("create shared memroy %d sucess\n", shmid);
	//man手册可以看到，system读取字符串，fork一个shell窗口执行字符串表示的指令
	system("ipcs -m");
	return 0;
}

