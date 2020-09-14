/*
 *	(GNU GPL) kobey
 *	测试一下踩内存
 *	踩内存，就是访问了不应该访问的内存地址
 *
 *	malloc调用libc库中的brk函数，brk函数是对系统调用brk的封装，
 *	brk申请堆空间的内存，是通过移动顶部指针进行扩展的，
 *	还有一个问题就是，如果我们申请1KB的内存，malloc实际上会申请132KB的内存
 *	malloc函数自身维护一些数据结构，还剩余一些将来可能用的上的内存空间
 *	所以，当我们第一次申请堆空间，然后故意越界访问，以为不会出问题
 *	但是，第二次申请的时候，就会出现踩内存，os并不知道我们第一次申请内存之后，就出现了
 *	越界，不会报错。
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void main()
{
	char *p = NULL;
	char *p2 = NULL;
	p = (char *)malloc(1*sizeof(char));
	strcpy(p,"hello");
	printf("%s\n", p);
	p2 = (char *)malloc(1*sizeof(char));
	strcpy(p2,"world");
	printf("%s\nend p = %s\n", p2, p);
	free(p);
	free(p2);
	p = NULL;
	p2 = NULL;
}




