/*	第二章、程序的执行成本
 *
 *	1、基准测试程序 benchmark test program
 *		衡量操作所需执行时间的程序，就是测试代码运行时间的测试程序
 *
 *	2、
 *
 *
 * */


#include "stdio.h"

void main()
{
	int a = 0, b = 1; long long n = 10000000000;

	printf("1\n");
	goto L1;
	printf("3\n");
	do {
L1:		printf("2\n");
		printf("2_1\n");
		goto L2;
		printf("5\n");
		a += b;
	} while( --n > 1000000000);
L2:	printf("4\n");
}
