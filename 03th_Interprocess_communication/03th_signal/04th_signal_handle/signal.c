/*
 *	(GNU v3.0) kobey
 *	signal函数，设置信号处理函数
 *
 *	一、函数原型：
 *	void (*signal(int signum, void (*handler)(int)))(int)；
 *		1，signal函数接受两个参数：
 *			int signum,表示响应信号的ID
 *			void (*handle)(int),表示自定义信号响应处理函数，接受int参数，其实就是信号ID
 *
 *		2，这个函数存在重叠，但是会以有效的最新处理方式为准，
 *		比如，在alarm之前，注册一个signal(14, ***),在alarm后的有效时间内可以重复注册
 *		其他的signal处理函数，然后，这个signal处理函数，将会以在alarm时间内最新的signal为准
 *	备注：每一个信号，都有默认的信号处理函数，也就是响应信号后的默认操作。
 *
 *	二、signal注册的处理函数有三种形式：
 *		1，SIG_IGN	忽略注册的信号响应，当啥事没有，继续往下走
 *		2，SIG_DFL	使用系统默认操作
 *		3，自定义函数
 * */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void sig_handle(int signum)
{
	for(int j = 0; j < 5; j++) {
		printf("j = %d\n", j);
	}
//	return;//return直接返回main函数继续之前中断后的下一条执行继续执行,不加return默认返回main。
//	//return返回主函数，如果没有主函数，就返回一个值，0 -1等。
}

void sig_handle1(int signum)
{
	printf("sig_handle1\n");
}
int main()
{
	signal(14, sig_handle);//自定义信号处理函数，但这个将自动忽略
	printf("alarm before\n");
	alarm(5);
	signal(14, sig_handle1);//这个也会自动忽略
	signal(14, SIG_IGN);//忽略该信号的响应
	signal(14, SIG_DFL);//使用默认的信号响应处理函数，比如14号，对应alarm定时信号，默认操作是终止
	for(int i =0; i<15; i++) {
		sleep(1);
		printf("i = %d\n", i);
	}
//	signal(14, sig_handle);//在信号的有效区间之后注册，就没有任何意义了！
	return 0;
}
