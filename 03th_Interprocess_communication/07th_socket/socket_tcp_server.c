/*
 *	（GNU v3.0）kobey 2020年08月12日18:03:44
 *
 *
 * */


#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
/*man bzero*/
#include <strings.h>

#include <stdlib.h>
/*  cat /proc/sys/net/ipv4/ip_local_port_range 
 *                        32768 60999
 */

#define SERVER_PORT	36666


int main()
{

	int serversocketfd;
	struct sockaddr_in server_addr,client_addr;
	int addr_len = sizeof(client_addr);
	int client;
	char buf[200];
	int idatanum;
	int ret1 = 0;
	printf("Start create a server socket ...\n");
	serversocketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(serversocketfd < 0) {
		printf("ERROR:can't create server socket\n");
		return -1;
	}
	printf("server socket create sucess\n");

	/*extern void bzero(void *s, int n); 
	 *	init server_addr
	 * */
	bzero(&server_addr, sizeof(server_addr));

	/*前面socket.c里面有谈到一个情况，需要转换主机字节序到网络字节序
	 *初始化服务器端的套接字，并用htons和htonl将端口和地址转成网络字节序
	 * */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	
	/*
	 *ip可以本服务器的ip，也可以用宏INADDR_ANY代替，代表0.0.0.0，表明所有地址
	 * */
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/*对于bind，accept之类的函数，里面套接字参数都是需要强制转换成(struct sockaddr *)
	 * */
	if(bind(serversocketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		printf("ERROR:server bind failure\n");
		return -2;
	}
	//设置服务器上的socket为监听状态
	if(listen(serversocketfd, 5) < 0) {
		printf("ERROR:server listen failure\n");
		return -3;
	}

	while(1) {
	
		printf("server listen port:%d\n", SERVER_PORT);

		/*	1,调用accept函数后，会进入阻塞状态
		 *	2,accept返回一个套接字的文件描述符，这样服务器端便有两个套接字的文件描述符
		 *		serversocketfd和client
		 *	3,serversocketfd仍然继续在监听状态，client则负责接收和发送数据
		 *	4,client_addr是一个传出参数，accept返回时，传出客户端的地址和端口号
		 *	5,addr_len是一个传入-传出参数，传入的是调用者提供的缓冲区的
		 *		client_addr的长度，以避免缓冲区溢出。
		 *		传出的是客户端地址结构体的实际长度
		 *	6,出错返回-1
		 *
		 * */
		client = accept(serversocketfd, (struct sockaddr *)&client_addr, (socklen_t *)&addr_len);
		if(client < 0) {
			printf("ERROR:accept failure\n");
			return -4;
		}
		printf("waiting msg ...\n");
		/*	1,inet_ntoa ip地址转换函数，将网络字节序IP转换为点分十进制IP
		 * */
		printf("IP is %s\n", inet_ntoa(client_addr.sin_addr));
		printf("Port is %d\n", htons(client_addr.sin_port));
		
		while(1) {
		
			printf("receive msg:");
			buf[0] = '\0';
			idatanum = recv(client, buf, 1024, 0);
			if(idatanum < 0) {
				printf("ERROR:receive null\n");
				continue;//返回循环体的判断条件，重新执行
			}

			buf[idatanum] = '\0';
			if(strcmp(buf, "quit") == 0)
				break;//收到quit，就退出循环体
			printf("%s\n", buf);
			printf("send msg:");
			scanf("%s", buf);
			printf("\n");
			send(client, buf, strlen(buf), 0);

			if (strcmp(buf, "quit") == 0)
				break;
		}
	
	
	}

	close(serversocketfd);
	return 0;



}	
