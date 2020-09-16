/*
 *
 *	使用glibc的钩子函数mtrace来检测代码级别的内存泄露
 *	<mcheck.h>中的mtrace钩子函数可以检测接下来的执行是否存在内存泄露
 *	在编译之前，需要做export MALLOC_TRACE=/dev/stdout，也就是log重定向到telnet终端
 *	在编译的时候，需要加上-g或者-ggdb调试信息
 *	编译后，直接执行，就可以看到内存申请以及是否存在释放的情况（+申请，-释放）
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <mcheck.h>

int main()
{
//	mtrace();
//	char *p = (char *)malloc(20);

//	free(p);
//	p = NULL;
	return 0;
}
