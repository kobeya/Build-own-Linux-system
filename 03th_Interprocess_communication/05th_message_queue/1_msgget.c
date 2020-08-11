/*
 *	(GNU v3.0)kobey 2020年08月11日14:06:55
 *	347511974@qq.com
 *
 *	【进程间通讯-消息队列1】
 *
 *	【实现原理】
 *		在前面，我们实现了
 *		无名管道	pipe			只能亲缘进程间通讯的管道缓冲区
 *		有名管道	mkfifo			可以实现非亲缘进程间通讯的文件节点
 *		信号通讯	kill、raise、alarm	利用内核自带的信号体系
 *		共享内存	shmget、shmat		创建一段共享内存
 *
 *		现在我们实现
 *		消息队列	msgget			创建一个用链表来组织数据的消息队列
 *
 *		一、函数原型
 *		 int msgget(key_t key, int flag);
 *		 key与共享内存一样，有两种选择，ftok、IPC_PRIVATE；
 *		 flag与共享内存一样，也表示创建的共享介质的权限属性。
 *		 成功，返回消息队列的ID，失败返回-1。
 *
 *		二、创建的消息队列数据结构演示
 *		——————————————————————————————————————————————————————————————————————————————
 *		内核	msgid_ds() ->消息队列的数据结构体		用链表为管理消息队列
 *			|							|
 *		msgid->	|						(中间有n个消息)
 *			|msg_first ---->指向第一个消息【口】-------------------------------|
 *			|								   :
 *			|msg_last -------------------------------------->指向最后一个消息【口】
 *			|
 *			|
 *		内核	|
 *		——————————————————————————————————————————————————————————————————————————————
 *
 *		三、用户空间的查看工具
 *		与共享内存一样，ipcs -q，稍后我们也可以看到，这个工具的实现api与共享内存
 *		原理是一样的，共享内存使用shmctl，设置不同的参数，就实现不同的功能。
 *		消息队列使用msgctl，也是设置不同的参数，实现不同的功能
 *		
 *
 *		三、发送、接收、控制 API
 *		
 *		1，发送(写)
 *			 int msgsnd(int msqid, const void *msgp, size_t size, int flag)；
 *			 msgid，消息队列ID
 *			 msgp，指向消息队列的指针
 *			 	一般消息成员的数据结构是这样的
 *			 	struct msgbuf {
 *					long mtype;//消息类型,因为链表中可以存放不同的数据类型。
 *					char mtex[N];//消息正文
 *			 	};
 *			 size，发送的消息正文数量，单位字节
 *			 flag，
 *			 	IPC_NOWAIT:消息没有发完函数也会立即返回；
 *			 	0:直到消息发完函数才会返回。
 *			 成功返回0，失败返回-1。(write函数成功会返回实际写的字节数)
 *
 *		2，接收
 *			int msgrcv(int msgid, void *msgp, size_t size, long msgtype, int flag)；
 *			msgid，消息队列的ID
 *			msgp，接收消息的缓冲区
 *			size，要接收消息的字节数
 *			msgtype，
 *				0，接收消息队列的第一个消息
 *				>0，接收消息队列中第一个类型为msgtype的消息
 *				<0，接收消息队列中类型值不大于msgtype的绝对值切类型值又最小的消息
 *			flag，
 *				0：若无消息函数一直阻塞
 *				IPC_NOWAIT，若没消息，进程立即返回ENOMSG
 *			成功返回接收到的消息长度，失败返回-1。
 *
 *			
 *		3，控制
 *			int msgctl(int msgqid, int cmd, struct msgqid_ds *buf)；
 *			msgid，消息队列ID
 *			cmd，
 *				IPC_STAT，读取消息队列的属性，并将其保存在buf指向的缓冲区
 *				IPC_SET，设置消息队列属性，这个值取自buf参数
 *				IPC_RMID，从系统中删除消息队列
 *			buf，消息队列缓冲区
 *			成功返回0，失败返回-1。
 *			
 *
 *
 *	【特征】
 *		一、消息队列的数据结构是链表，写就是插入数据，读就是删除数据，所以，
 *			消息队列跟管道文件一样，读完就消失了，无法二次读取。
 *
 *		我们也可以总结：队列的特性，读就是出列，出了列队，该单元在队列中就没了，
 *		再去读，当然读不到，我们可以设置为阻塞或者非阻塞！
 *
 *
 *		二、 在前面我们讨论共享内存的时候，不过是哪个进程读写了共享内存，都需要发一个信号
 *		给其他进程去唤醒进程事务。
 *			但是，消息队列不需要，理由有两个：
 *				① 消息队列可以设置读写自我阻塞，也就是他们在没有写或者没有读的时候
 *				自动休眠，有读或者写的时候，自动唤醒。flag参数的作用。
 *
 *				② 消息队列在读之后，出队，删除了该队列单元
 *				通俗一点描述就是消息队列读端阻塞读，没有得读就自我阻塞，有得读就
 *				一直读，并且读的过程也是删除的过程。
 *				消息队列阻塞写，一直写完才会返回，否则就一直写，写到没空间就
 *				自我阻塞。
 *			这里插播一下共享内存需要发信号的原因，我们可以思考一下，某个进程写完共享内存
 *			，此刻其他进程怎么知道这个共享内存已经被写了呢？它不像我们消息队列一样，
 *			读处于阻塞状态，共享内存，读完就不再等待了。所以，我们的共享内存，需要一个
 *			信号去唤醒它。这个信号怎么发送，有两个选择，父子进程直接可以通讯，
 *			没有亲缘关系的两个进程，就通过增加一个信号通讯，信号通信需要pid资源。
 *
 *
 * */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>


int main()
{

	int msgid = -1;
	int ret = -1;
	msgid = msgget(IPC_PRIVATE, 0777);
	if(msgid < 0) {
		printf("msgget failure\n");
		return -1;
	}
	system("ipcs -q");
	/*删除消息队列*/
	ret = msgctl(msgid, IPC_RMID, NULL );
	if(ret < 0){
		printf("msgctl delect failure\n");
		return -2;
	}
	
	system("ipcs -q");
	return 0;
}
