/*
 *	(GNU v3.0) kobey 2020年08月10日08:17:47
 *	进程间通讯3——共享内存
 *
 *	Q，前面提到，在创建内核态的共享内存时，key的标志位使用IPC_PRIVATE，如果使用
 *		key = ftok(*, *)来创建共享内存，有什么差别？
 *	A，在使用shmget函数创建共享内存时，使用的这两个不同的key字段，功能相当于无名管道
 *	和有名管道的进程间通讯效果：
 *		IPC_PRIVATE来创建共享内存，只能有亲缘关系的进程间通信
 *		key = ftok()来创建的共享内存，可以实现任意进程间通信。
 *	ftok函数原型：
 *		key_t ftok(const char *pathname, int proj_id);
 *
 *
 *	[shmat]
 *		在这之前，我们创建的共享内存，都是处于内核态，我们可以使用shmat这个API把内核空间
 *		中的共享内存，映射到用户空间，这样就可以减少内核空间的访问，而且可以使用用户空间
 *		的open read write接口做常规文件读写操作了。
 *
 *		void *shmat(int shmid, const void *shmaddr, int shmflg) //类似于malloc
 *		参数列表：
 *		shmid，共享内存的ID号
 *		shmaddr，映射到用户空间后的地址，如果要系统自动分配，则填入NULL
 *		shmflg，共享内存权限字段，SHM_RDONLY，只读； 0，表示可读写。
 *
 *		函数成功，返回指向映射后的地址；
 *		失败，返回NULL。
 *
 *	备注：共享内存，不像管道文件，读完就消失了，再读就会出现读阻塞。共享内存，可以重复读取，
 *	直到删除(shmdt,shmctl)或者系统关闭。
 *
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
	//man手册可以看到，system读取字符串，fork一个shell窗口执行字符串表示的指令
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
	return 0;
}

