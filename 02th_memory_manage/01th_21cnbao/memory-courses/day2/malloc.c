#if 0
引用：https://www.cnblogs.com/zzdbullet/p/9635318.html
malloc()和free()的原理实现

	1，malloc申请的内存在哪？
		进程用户态的堆空间

	2，堆空间怎么管理？
		堆空间使用空闲链表来管理各种大小的内存块
		内存块本身也使用一个数据结构：
		struct mem_control_block {
			int is_available;//标志块是否已经使用
			int size;//这个字段描述块的大小
		};

	3，malloc分配内存的实际大小？
		malloc实际上除了分配用户需要的内存大小外，还需要一些自己相关的额外的管理内存。
		实际大小 = 管理空间（用户看不到） + 需求空间
	
	4，malloc的管理空间是怎么回事？
		在64bit系统中，malloc(0)的有效内存大小为24，在32bit中为12。
		准确来说，至少是这么多，并且这些内存是可用的。
		分配的实际大小可用这么算：

		在64位的系统中，
		A 如果申请的内存块少于等于24Byte，那么申请的实际大小等于24Byte
		B 如果大于24Byte，那么就按照以24byte为底，16byte对齐增加的方式申请，
			比如，申请25byte，
			那么申请到的内存为
			24 + 16 = 40byte

	5，描述一下malloc的工作原理？
		首先挨个检查堆中的内存是否可用，如果可用那么大小是否能满足需求，要是都满足的话就
		直接用。当遍历了堆中的所有内存块时，要是没有能满足需求的块时就只能通过系统调用
		向操作系统申请新的内存，然后将新的内存添加到堆中。

	6，描述下free？
		取得相应内存块的首地址，将该块的is_available设为可用。




#endif



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main ()
{
	char *p = (char *)malloc(0);
	char *p1 = (char *)malloc(5);
	char *p2 = (char *)malloc(25);
	char *p3 = (char *)malloc(24);
	char *p4 = (char *)malloc(57);

	printf("p size = %d\n", malloc_usable_size(p));
	printf("p1 size = %d\n", malloc_usable_size(p1));
	printf("p2 size = %d\n", malloc_usable_size(p2));
	printf("p3 size = %d\n", malloc_usable_size(p3));
	printf("p4 size = %d\n", malloc_usable_size(p4));

	/*这里需要注意，只用于演示，实际使用的时候，应该加上一个返回非NULL的判断*/
	if(p)
	{
		printf("malloc p mem sucess!\n");
	}
	
	/*验证一下malloc申请的虚拟内存在没有写之前，是否清零*/
	int buf[1] = {6};
	printf("before read p,buf = %d\n", buf[0]);

	memcpy(buf, p, 1);

	printf("after copy p then buf = %d\n", buf[0]);


	free(p);free(p1);free(p2);free(p3);free(p4);
	while(1);
	return 0;
}
