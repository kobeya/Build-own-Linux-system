/*
 *	简单的测试用例
 *	1，strac -tt的使用
 *	2，LD_DEBUG=libs的使用，glibc为了调试自身的一个环境变量
 *
 * */

#include <stdio.h>
//unused libs
#include <string.h>
#include <unistd.h>

int main()
{
	printf("Hello, world!\n");
	return 0;
}
