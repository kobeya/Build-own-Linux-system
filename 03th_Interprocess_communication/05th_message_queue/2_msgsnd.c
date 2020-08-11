/*
 *	(GNU v3.2)kobey 2020年08月11日15:57:41
 *	【进程间通讯-共享内存2-读写演示】
 *
 * */


#include "sys/types.h"
#include "sys/msg.h"
#include "signal.h"
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

struct msgbuf {
	long msgtype;
	char buf[160];
};


int main()
{

	struct msgbuf sendbuf, rcvbuf;
	int key = -1;
	int msgid;
	int readnum = 0;
	key = ftok("./key", 'y');
	if(key < 0) {
		printf("create key failure\n");
		return -1;
	}

	msgid = msgget(key, IPC_CREAT | 0777);//不需要参数来描述队列的大小，因为链表的大小不固定
	if(msgid < 0) {
		printf("msgget failure\n");
		return -2;
	}

	system("ipcs -q");
	//init sendbuf
	sendbuf.msgtype = 100;
	printf("sendbuf strlen = %ld\n", strlen(sendbuf.buf));
	printf("please input message:\n");
	fgets(sendbuf.buf, 160, stdin);
	//write buf and send it
	msgsnd(msgid, (void *)&sendbuf, strlen(sendbuf.buf), 0);

	//init rcvbuf
	memset(rcvbuf.buf, 0, 160);
	
	readnum = msgrcv(msgid, (void *)&rcvbuf, 160, 100, 0);
	printf("rcv:%s\n", rcvbuf.buf);
	printf("read number = %dBytes\n", readnum);

	msgctl(msgid, IPC_RMID, NULL);
	system("ipcs -q");
	return 0;
}



