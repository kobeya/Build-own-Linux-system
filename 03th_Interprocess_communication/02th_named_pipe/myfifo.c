/*	(GNU v3.0) kobey 2020年08月10日09:34:26
 *	有名管道
 *	缘由：
 *		无名管道，只能实现，亲缘关系的进程间通讯，无法实现两个不相干的进程间通讯。
 *		因为无名管道，只有文件描述符，在VFS中没有文件节点，没有亲缘关系的进程无法
 *		访问无名管道。
 *		有名管道，最简单的就是解决VFS没有文件节点的问题！
 *
 *	备注，VFS中，文件包含以下类型：
 *	① 普通文件	“-”
 *	② 目录		“d”
 *	③ 链接文件	“l”
 *	④ 管道文件	“p” //这个就是我们有名管道的文件节点，|
 *	⑤ 字符设备	“c”				      |____这四个文件，都不占用磁盘空间，
 *	⑥ 块设备	“b”				      |只是一个文件节点，作为文件的索引
 *	⑦ 套接字	“s”				      |		
 *
 *
 *	API原型：
 *		int mkfifo(const char *filename, mode_t mode);
 *							|		
 *							V
 *				umask(权限掩码):O_RDONLY,O_WRONLY,O_RDWR,O_CREATE
 *
 *		创建成功则返回0，失败返回-1。
 *
 *
 *	原理：
 *		mkfifo，先进入内核，内核在文件系统中生成文件节点，但是此刻内核里面是没有管道的，
 *		只有在调用open之后，内核才创建管道并且打开端口，并且通过文件节点进行有名通讯。
 *
 *
 *
 *	特点：
 *		一、可以实现完全没有亲缘关系的两个进程间通讯
 *		二、想通讯的两个进程，比如同时打开有名管道的文件节点，否则单个进程会陷入阻塞
 *
 *
 *
 * */


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
int main(int argc, char *argv[])
{
	int fd;
	fd = mkfifo("./myfifo", 0777);
	if(fd < 0) {
		printf("create myfifo failure\n");
	}
	printf("create myfifo sucess\n");
	return 0;
}

