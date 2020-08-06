#include <malloc.h>
#include <sys/mman.h>

#define SOMESIZE (100*1024*1024)	// 100MB

int main(int argc, char *argv[])
{
	unsigned char *buffer;
	int i;

	if (!mlockall(MCL_CURRENT | MCL_FUTURE))
		mallopt(M_TRIM_THRESHOLD, -1UL);
	mallopt(M_MMAP_MAX, 0);

	buffer = malloc(SOMESIZE);
	if (!buffer)
		exit(-1);

	/* 
	 * Touch each page in this piece of memory to get it
	 * mapped into RAM
	 */
	for (i = 0; i < SOMESIZE; i += 4 * 1024)
		buffer[i] = 0;
	free(buffer);
	/* <do your RT-thing> */

	while(1);
	return 0;
}


/*详解这个程序用到的重要知识体*/

#if 0

1，C语言支持两种内存申请
	A 静态申请，static 或者全局变量，都是静态申请内存，生命周期包含在整个程序体。
	B 自动申请，函数参数+局部变量，生命周期跟随函数体的代码块。

2，库函数（宏）支持的内存申请
	属于动态内存申请！
	example，malloc函数原型：void *malloc(size_t size);
	A malloc函数申请的内存，不会初始化，需要调用，memset函数进行初始化。
		void *memset(void *block, int c, size_t size)
	  malloc申请的内存自动对齐：32位系统以8倍对齐，64位以16的倍数对齐。

	B 如果你想为字符串申请内存，记得大小为长度+1——字符串的最后的空字符并没有被计入长度
		char *p;
		ptr = (char *) malloc(sizeof(length + 1));

	
	在malloc的基础上，还想继续增加这么内存块，也就是调整动态申请的内存块的大小，
	使用realloc函数：
		void *realloc(void *ptr, size_t newsize);
	这个函数的工作原理：
		首先，从原来的内存块的基于malloc分配的内存，继续增大分配出去的内存块，
		如果，那个供分配的内存块不够大，就会重新找一个足够大的空间，申请newsize大小的内存，
		并将旧内存复制到新内存中。










#endif
