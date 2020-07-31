# memory-courses :

## 硬件原理和分页管理

*   CPU寻址内存，虚拟地址、物理地址
*   MMU以及RWX权限、kernel和user模式权限
*   内存的zone: DMA、Normal和HIGHMEM
*   Linux内存管理Buddy算法
*   连续内存分配器(CMA)

## 内存的动态申请和释放

*   slab、kmalloc/kfree、/proc/slabinfo和slabtop
*   用户空间malloc/free与内核之间的关系
*   mallopt
*   vmalloc
*   内存耗尽（OOM）、oom_score和oom_adj
*   Android进程生命周期与OOM

## 进程的内存消耗和泄漏

*   进程的VMA。
*   进程内存消耗的4个概念：vss、rss、pss和uss
	1，vss
		虚拟地址空间，virtual set size。一般在我们创建进程，malloc虚拟地址空间的时候增大
		包含共享库占用的内存
	2，rss
		驻留在内存条上的内存,也就是实际使用的物理内存
		包含共享库占用的内存
	3，pss
		实际使用的物理内存
		比例分配共享库的占用内存
	4，uss
		进程独自占用的物理内存
		不包括共享库占用的内存

	一般 vss >= rss >= pss >= uss

	可以使用第三方的工具包，编译出来的工具观察进程的内存分配情况：procrank
	
	
*   page fault的几种可能性，major和minor
*   应用内存泄漏的界定方法
*   应用内存泄漏的检测方法：valgrind和addresssanitizer

## 内存与I/O的交换

*   page cache
*   free命令的详细解释
*   read、write和mmap
*   file-backed的页面和匿名页
*   swap以及zRAM
*   页面回收和LRU

## 其他工程问题以及调优

*   DMA和cache一致性
*   内存的cgroup
*   性能方面的调优：page in/out, swapin/out
*   Dirty ratio的一些设置
*   swappiness
