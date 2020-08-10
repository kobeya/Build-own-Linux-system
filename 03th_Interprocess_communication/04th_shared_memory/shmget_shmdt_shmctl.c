/*
 *	(GNU v3.0) kobey 2020年08月10日09:53:13
 *	进程间通讯4——共享内存
 *	备注：共享内存，不像管道文件，读完就消失了，再读就会出现读阻塞。共享内存，可以重复读取，
 *	直到删除(shmdt,shmctl)或者系统关闭。
 *
 *	前面看到了，共享内存的删除ipcrm -m shmid来实现，那么这个工具是怎么实现的？
 *	删除一个共享内存可以分为两种：
 *	一、删除用户空间的共享内存映射，使用
 *	shmdt,
 *		 int  shmdt(const void *shmaddr);
 *		 参数： shmaddr共享内存映射后的用户地址，也就是shmat的返回值。
 *		 成功，返回0，失败返回-1。
 *	二、删除内核空间的共享内存，使用
 *	shmctl，
 *		int shmctl(int shmid, int cmd, struct shmid_ds * buf)；
 *		参数：	shmid，要操作的共享内存标识符
 *			cmd：
 *				IPC_STAT，获取对象属性值 //实现了命令ipcs  -m
 *				IPC_SET，设置对象属性
 *				IPC_RMID，删除对象 //实现了命令ipcrm -m
 *			成功，返回0，失败返回-1。
 *
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
	int key;
	char *p;
	key = ftok("./key", 'k');
	if(key < 0) {
		printf("ftok failure\n");
		return -2;
	}
	shmid = shmget(key, 16,IPC_CREAT | 0777);
	if(shmid < 0) {
		printf("create shmget failure\n");
		return -1;
	}

	printf("create shared memroy %d sucess\n", shmid);
	//man手册可以看到，system读取字符串，fork一个shell子进程窗口执行字符串表示的指令
	system("ipcs -m");

	p = (char *)shmat(shmid, NULL, 0);
	if(p == NULL)
	{
		printf("shmat memory failure\n");
		return -3;
	}

	fgets(p, 16, stdin);
	printf("share memory data = %s", p);//注意这里留个细节，没有换行符，此刻也会自动换行
	//second read
	printf("second read\tshare memory data = %s\n", p);

	/*删除用户空间的映射*/
	int ret1 = -1;
	ret1 = shmdt(p);
	if(ret1 < 0) {
		printf("shmdt failure\n");
		return -4;
	}
	printf("shmdt sucess,and share memory user space detect!\n");
	int ret2 = -1;
	ret2 = shmctl(shmid, IPC_RMID, NULL);
	if(ret2 < 0) {
		printf("shmctl failure");
		return -5;
	}
	printf("shmctl sucess,and share memory kernel space detect!\n");
	system("ipcs -m");

	return 0;
}

