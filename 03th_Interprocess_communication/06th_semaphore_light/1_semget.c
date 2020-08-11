/*
 *	(GNU v3.0)kobey 2020年08月11日17:56:24
 *
 *	进程间通讯——信号量（信号灯）semaphore
 *
 *	【框架、原理】
 *		IPC:共享内存、消息队列、信号量
 *	
 *		创建一个信号量集合来实现进程间通讯。
 *
 *	Q，进程间通讯的信号量与多线程posix中的信号量有什么区别？
 *	A，在多线程posix里面说的信号量，是单个信号量；
 *		进程间通讯使用的信号量是信号量的集合，也就是不是单个信号量。
 *		但是他们的原理是一样的。
 *		posix信号量的PV操作
 *			① 定义一个信号量sem_t sem
 *			② 初始化信号量sem_init
 *			③ p（sem_wait）或者v（sem_post）操作
 *
 *		IPC信号灯的PV操作,PV操作都在一个API里面通过第二个参数的某个字段变更实现PV操作
 *
 *		int semop(int semid, struct sembuf *opsptr, size_t nops);
 *		参数
 *		semid,信号灯，也就是信号量的集合ID
 *		struct sembuf{
 *			short sem_num;//遥操作的信号灯里信号量的编号，信号灯用数组来管理信号量
 *			short sem_op;//0,等待，直到信号灯的值为0,；1，释放操作，V操作；-1，P操作。
 *			short sem_flg;//0，IPC_NOWAIT阻塞, 1，SEM_UNDO非阻塞。
 *		};
 *
 *		nops,要操作的信号灯中信号量的个数。
 *
 *
 *
 *
 *
 *
 *		所以，要清醒一个概念，进程间通讯中的信号量是针对一个集合进行init，p操作和v操作。
 *
 *	
 *	一、创建、控制的API
 *		创建，
 *		int semget(key_t key, int signa_num, flag_t flag);
 *		key，跟共享内存，消息队列一样，有两个选择：
 *			IPC_PRIVATE
 *			key = ftok(pathname, '*');
 *		signal_num,创建的信号灯里面包含的信号量的个数；
 *		flag，权限属性。
 *
 *		控制，
 *		int semctl(int semid, int semnum, int cmd, ...union semun arg);
 *		semnum,要修改的信号灯里面信号量的编号，信号灯里面的信号量是一个数组，也就是用
 *			数组的下标来表示该信号灯的某个信号量即可。
 *		cmd，
 *			GETVAL,获取信号灯的值
 *			SETVAL，设置信号灯的值
 *			IPC_RMID,删除信号灯集合
 *
 *		...union,设置的值，用联合体来描述
 *
 *		成功返回0，失败-1
 *
 *
 *	
 * */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <pthread.h>
#include <semaphore.h>
//信号灯头文件
#include <sys/sem.h>
#include <sys/ipc.h>


sem_t sem;//定义一个信号量

static int semid;//定义一个信号灯的ID

/*man semctl手册可以查到*/
union semun 
{	                
	int  val;  	/* Value for SETVAL*/
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */	                  
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO(Linux-specific) */
};
/*define a union*/

union semun mysemun;


struct sembuf mysembuf;



static void *fun(void *var)
{
	//子线程v操作，就算sleep定时，也会先比主线程先运行，因为主线程在P操作的等待V操作状态。
	sleep(1);
	sem_post(&sem);
	printf("child pthread running...\nread data = %s\n",var);
	return NULL;
}

static void *fun1(void *var)
{
        mysembuf.sem_op = 1;
	semop(semid, &mysembuf, 1);
        printf("IPC_sem child pthread running...\nread data = %s\n",var);
        return;
}

int main()
{
	int semid;
	semid = semget(IPC_PRIVATE,3, 0777);
	if(semid < 0) {
		printf("semget failure\n");
		return -1;
	}
	system("ipcs -s");
	semctl(semid, 3, IPC_RMID, NULL);
	system("ipcs -s");



	/*创建一个多线程的例子*/
	int ret;
	pthread_t tid;
	char *str = "hello world";
	/*初始化一个信号量
	 * 第一个参数，表示需要初始化信号量的地址
	 * 第二个参数，0，表示用于多线程，非0用于进程
	 * 第三个参数，对这个信号量初始化的值
	 * */
	sem_init(&sem, 0, 0);//sem = 0

	ret = pthread_create(&tid, NULL, fun, (void *)str);
	if(ret < 0)
	{
		printf("pthread create failure\n");
		return -2;
	}
//	sleep(1);
	/*使用信号量来阻塞主线程，让子线程先跑
	 * 	p操作，wait
	 * */
	sem_wait(&sem);//此刻，主线程就会阻塞

	printf("main pthread running...\n");
	if(pthread_join(tid, NULL)) {
		printf("child pthread free ERROR!\n");
		return -3;
	}
	printf("child pthread free sucess\n");

	printf("posix sem done !");

	sleep(1);

	printf("IPC sem start...\n");

	//使用信号灯来实现
	semid = semget(IPC_PRIVATE, 4, 0777);
	if(semid < 0)
	{
		printf("semget failure\n");
		return -4;
	}
	printf("semget create sucess, semid = %d\n", semid);
	system("ipcs -s");
	/*init*/
	mysemun.val = 0;
	if(semctl(semid, 0, SETVAL, mysemun)){printf("semctl init error\n");}
	mysembuf.sem_num = 0;
	mysembuf.sem_flg = 0;
	int ret1;
	ret1 = pthread_create(&tid, NULL, fun1, (void *)str);
	printf("IPC sem: main pthread sleep...\n");
	mysembuf.sem_op = -1;
	semop(semid, &mysembuf, 1);
	printf("IPC sem:main pthread runnig...\n");
	if(pthread_join(tid, NULL)){printf("pthread join ERROR");return -5;}
	system("ipcs -s");
	return 0;
}
