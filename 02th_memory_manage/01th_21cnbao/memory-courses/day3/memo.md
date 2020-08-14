主题：进程的内存消耗和泄露

一、摘要

	1，进程的虚拟地址空间VMA（Virtual Memory Area）

	2，Pagefault的几种可能性、VMA的作用、major缺页和minor缺页

	3，进程内存消耗的4个概念：vss、rss、pss和uss

	4，应用内存泄漏的界定方法

	5，内存泄漏的检测工具：valgrind和addresssanitizer

二、进程的虚拟地址空间VMA
	

	1，数据结构
			task_struct {
				...
				mm_struct ----> mm_srtuct---->这个数据结构，代表了进程整个内存资源
			};				count;
							pgd;--->页表
							...
							mmap;--->这个指针指向vm_area_struct链表
									|
									V
								vm_area_struct
			vm_area_struct链表的每一个节点，		vm_end;
			都代表进程中的一个虚拟地址空间VMA               vm_start;
									vm_flags;
									vm_inode;
									vm_ops;
									...
						vm_area_struct <--------vm_next;
							...
							vm_next
			描述：
			进程中的mm_struct数据结构，描述了进程的所有内存资源。
			其中，mm_struct有一个mmap的指针，指向一个VMA的链表，链表的每个节点，都表示
			一个VMA，这些VMA最终对应ELF可执行程序的数据段、代码段、堆栈、共享库等区域。

	2，用户空间，查看VMA的分布情况
		有三种方法:
			pmap工具
			cat /proc/pid/maps
			cat /proc/pid/smaps
			这里演示第二种方法
			kobey@pc:~$ cat /proc/17980/maps 

5615a53a9000-5615a53aa000 r-xp 00000000 08:01 11026946            /home/kobey/a.out
5615a55a9000-5615a55aa000 r--p 00000000 08:01 11026946            /home/kobey/a.out
5615a55aa000-5615a55ab000 rw-p 00001000 08:01 11026946            /home/kobey/a.out
7f85af31a000-7f85af501000 r-xp 00000000 08:01 17039550            /lib/x86_64-linux-gnu/libc-2.27.so
7f85af501000-7f85af701000 ---p 001e7000 08:01 17039550            /lib/x86_64-linux-gnu/libc-2.27.so
7f85af701000-7f85af705000 r--p 001e7000 08:01 17039550            /lib/x86_64-linux-gnu/libc-2.27.so
7f85af705000-7f85af707000 rw-p 001eb000 08:01 17039550            /lib/x86_64-linux-gnu/libc-2.27.so
7f85af707000-7f85af70b000 rw-p 00000000 00:00 0 
7f85af70b000-7f85af732000 r-xp 00000000 08:01 17039546            /lib/x86_64-linux-gnu/ld-2.27.so
7f85af8fc000-7f85af8fe000 rw-p 00000000 00:00 0 
7f85af932000-7f85af933000 r--p 00027000 08:01 17039546            /lib/x86_64-linux-gnu/ld-2.27.so
7f85af933000-7f85af934000 rw-p 00028000 08:01 17039546            /lib/x86_64-linux-gnu/ld-2.27.so
7f85af934000-7f85af935000 rw-p 00000000 00:00 0 
7fff52a4b000-7fff52a6c000 rw-p 00000000 00:00 0                   [stack]
7fff52bf3000-7fff52bf6000 r--p 00000000 00:00 0                   [vvar]
7fff52bf6000-7fff52bf7000 r-xp 00000000 00:00 0                   [vdso]
ffffffffff600000-ffffffffff601000 --xp 00000000 00:00 0           [vsyscall]

		我们可以看到
			① 每个VMA的地址分配，大小
			② 整个进程的内存地址中间会出现很多空白区域，空白区域对于进程来说是不能访问的
			③ 每个VMA的访问权限
			
		

二、page fault的几种可能性、VMA的作用、major缺页和minor缺页
	可以参考这个帖子：https://csdnnews.blog.csdn.net/article/details/107171732
			 https://www.it610.com/article/1281519089482743808.htm
	1，page fault的几种可能性(类型与原因)
		① minor缺页，也称呼为软缺页、次要缺页。物理内存中，存在对应页帧，无需从磁盘
		拷贝数据，只不过没有建立映射关系，或者没有对应的访问权限。此时，MMU只需要重新建立
		映射关系，修改页表即可。
		比如，写只读的内存、写malloc分配的内存(因为lazy行为，没有建立实际映射)

		② major缺页，也称呼为主缺页、硬缺页。在物理内存中，没有对应的页帧，需要CPU打开
		磁盘设备，读取到物理内存，然后再让MMU建立VA和PA的映射。
		比如，swap数据置换(访问swap分区的页面，会触发page fault)。

		③ Invalid缺页，也称呼为无效页～缺页异常。访问不存在或者禁止访问的内存地址。
		比如，内存地址越界访问(禁止访问)、对空指针解引用(不存在)，内核就会发出段错误，
		segment fault,进程直接挂掉！

	
		
	2，出现page fault后，怎么处理？(VMA的作用)
		针对第一类minor缺页，MMU给CPU发出page fault，MMU可以从硬件寄存器读出发生page fault
		的地址和发生原因，内核收到缺页中断，根据缺页的原因和地址，去VMA中查，发现真实的原因，
		然后对应类型的page fault异常处理程序就会执行。

		举个实例，写malloc分配的内存，page fault，后续怎么处理：
		因为由于内存分配的lazy行为，malloc后，并没有建立实际的VMA与PA的映射关系，也就是没有
		获取到真实的物理内存，内核只是把所有要分配的页表都映射到同一片已经清零的物理地址，
		并标志页表为ReadONLY，但是，当malloc返回的时候，对应的heap的VMA区域已经产生了，
		且已经进入了内核管理的vm_area_struct链表中了，且权限被标志为读写。
		当我们向这片内存写的时候，发现页表权限只读，
		此刻，触发page fault。内核收到异常信号，就会到硬件寄存器(MMU)
		中知道发生的原因和地址，知道是访问的虚拟内存地址没有实际映射，然后就会到VMA查询，
		发现，访问的虚拟内存是有写权限的，确实没有实际映射。page fault handler就会真正的申请
		物理内存，修改VMA中的页表，建立映射，修正权限。


		malloc page fault这个异常的时序演示：
	

		malloc ----> heap中申请----一块内存，此刻做三个事情---->① 映射到同一片清零的PA
						|		|	
						V		|
					② 修改页表，只读	|
								V
							③ 产出VMA，但权限是读写

		wirte这块内存--->发现② 只读--->告诉内核--->内核检查VMA，发现读写
								|
								V
					再实际分配可读写的物理内存，建立映射，修改页表！	




	总结：
		VMA记录了内存的真实属性，page fault后，Linux会查询VMA和页表的权限，提供信息给内核，
		执行响应的处理程序。


三、进程消耗的4个概念:vss、rss、pss、uss
	
	备注：我们评估一个进程的内存消耗，都是指用户空间的内存，不包括内核空间的内存消耗！

	1，VSS是进程看到的，自己在虚拟内存空间所占用的内存
	2，RSS是进程实际真正使用的内存
	3，PSS是多进程共享一片内存的容量取平均值，再加上自己独占的内存
	4，USS是进程所独占的内存容量
		
	
	5，评估进程内存消耗情况一般使用smem工具
		安装：
			sudo apt install smem
		使用：
			① 命令行模式 smem
			② smem –pie=command，smem –bar=command(收藏，我试了下不行，暂时不处理)
			


四、应用程序，内存泄露的界定方法
	
	1，内存泄露的定义:从现象来看，一个程序运行的时间越长，耗费的内存越多，申请与释放不成对，
			内存呈现震荡发散的情况，就说明这个程序内存泄露。

	2，判断内存泄露的数据来源:只需要看USS(你有可以看RSS)即可，就是进程所占用的内存容量。
		|
		|	USS、RSS
		|
		|					   _________________震荡发散，泄露了！
		|		      ____________________/	
		|		      /	
		|____________________/
		|
		|__________________________________________________________________> time




五、内存泄露的检测工具: valgrind、addresssanitizer
	在介绍之前，我们可以用前面描述到的smem来做一个手动的测试，
	比如反复多次执行smem -P ./a.out来采集./a.out执行的RSS或者USS的使用状态。

	1, 安装valgring:
		sudo apt install -y valgrind
	2，使用valgrind(将程序运行于一个虚拟机中，速度很慢):
		valgrind --tool=memcheck --leak-check=yes ./a.out
		执行一段时间后，CTRL+C发送SIGINT的信号去调用软中断终止进程运行，
		然后自动打印出检测的程序的内存泄露情况，
		比如关键字段：
		==15174== HEAP SUMMARY:
		==15174==     in use at exit: 331,776 bytes in 27 blocks
		==15174==   total heap usage: 54 allocs, 27 frees, 359,424 bytes allocated
		从这里，我们可以看到在堆空间中，申请了54次，但是只释放了27次，
		再往下，还可以看到发生泄露leak的程序名称。

	3，使用addresssanitizer(修改程序，重新编译，但速度很快，所以现在更加流行！)
		① 需要在代码中添加一个__lsan_do_leak_check做检测
			#include <sanitizer/lsan_interface.h>
			__lsan_do_leak_check();
		② 使用高于4.9版本的gcc编译，添加参数选项
			gcc -g -fsanitize=address ***.c
		
	



