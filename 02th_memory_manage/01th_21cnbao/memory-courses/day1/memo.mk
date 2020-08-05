#主题：硬件原理和分页管理


第一节，资料阅读
	CPU如何访问到内存的？——mmu最基本的原理！
	http://mp.weixin.qq.com/s/SdsT6Is0VG84WlzcAkNCJA
	知识脉络：
	1，在MMU开启后，我们首先知道，Linux是支持MMU也支持没有MMU的，有个选项可以启动。
		CPU任何时候，发出的地址，都是虚拟地址！
	2，虚拟地址，发给MMU，然后MMU通过页表查找对应的物理地址，再访问实际的cache或者内存条。
	3，图解：
		

		CPU   ——————> VA虚拟地址 ——————> MMU(页表基址寄存器) ——————> 物理内存

	4，这个帖子对内存寻址，我个人观点是不怎么赞同的，如果有兴趣深查，我建议看<深入理解计算机系统
		这本书，对内存寻址有非常完美的解说，包括练习！这里就到此为止吧！
		如果结合其他段寻址的书籍，比如内核完全注释等，能收获更多。
		很烦的是，我看了n遍，包括练习反复做了，好像现在又有点忘记了，脑子真的不能太相信！
	
	用代码切身实践体会meltdown漏洞
	http://mp.weixin.qq.com/s/lJJU3LCepJgNq5AxyFFM8Q
	知识脉络：
	目前理解的整个思路，是利用CPU设计的硬性缺陷，分支预测，乱序执行的特性，
	用户空间盗取内核空间的数据。
	
				    分段		分页
	5，虚拟地址（逻辑地址）————————————>线性地址————————————>物理地址
				（段寄存器）	    （MMU，页表）	

	  段选择符+段内偏移量		各级页表索引+偏移量	PPD+偏移量


	6，虚拟内存系统架构VMSA
	VMSA的基本思想，在程序的大小超过物理内存大小的时候，OS就把当前用到的程序送到内存，暂时
	不用的就保存在磁盘，在需要用到的时候，再做程序片段的交换。
	VMSA架构，需要MMU来实现虚拟内存与物理地址的映射转换。


要点：
*   CPU寻址内存，虚拟地址、物理地址
*   MMU以及RWX权限、kernel和user模式权限
*   内存的zone: DMA、Normal和HIGHMEM
*   Linux内存管理Buddy算法
*   连续内存分配器(CMA)
	1，MMU是什么？
		一般，我们可以知道，从80386开始，设置寄存器CR0的PG位，可以控制分页机制，
		PG = 1，启动分页，PG = 0，直接就是物理地址寻址。简单的说，MMU就是一个负责
		地址转换的，基于基地址寄存器CR0的一个硬件设备。
			
		百度上找到关于ARM系列的MMU解释：
		MMU是作为一个编号为15的协处理器的角色存在，ARM mmu提供1k/4K/64K，3种模式。
		涉及的寄存器，全部都位于协处理器15。
		
		找到一个帖子，关于MMU操作的实例描述：
		http://blog.chinaunix.net/uid-69947851-id-5826790.html

		核心工作：
		通过协处理器的各种寄存器，初始化MMU，写页表映射到TTB0寄存器等操作。


	2，MMU做了哪些工作？
		① 提供虚拟地址到物理地址的转换
			这里简单说一下过程，分段后的线性地址，通过CR3寄存器保存的页表基地址
			找到页表，页表寻址，到物理地址
		② 地址访问权限的保护
			每一个页表项中，都有RWX的标志，U/K标志，访问越权就会page fault


	3，虚拟地址本质，是一个寻找物理地址的指针！而物理地址，在内核中的定义是一个整数！！！

	4，由于页表访问速度很慢，引出MMU的核心部件TLB，TLB中缓存着少量转换表，也就是页表的cache。
	5，当TLB没有命中，引出MMU另外一个核心部件TTW（Translation Table Walk），转换表漫游。
		当在TTW中转换成功，也应该把结果写入TLB中。
	6，MMU还有另外一个核心部件：MPU，内存保护单元，做权限管理，不负责虚实映射！

		截止到现在，我们可以总结：
						     |---TLB
				VA  ————————————>MMU |---TTW   ————————> PA
						     |---MPU	

	
	
	7，内存区域
	简述分区：
	① 首先知道，分区是没有任何物理意义的，只不过是内核为了管理页而采用的一种逻辑分组！
	② 为什么出现ZONE_DMA?
	答：比如x86 ISA总线上的DMA引擎在访问内存的时候，地址线只能发到16MB以下，它的硬件特性
	注定它无法访问内存16MB以上的物理内存。内核为了管理这样的情况，划分16MB的区域为DMA_ZONE。
	如果，某些体系架构，所有的内存都可以进行DMA操作，那么这个ZONE_DMA就为空！
	还需要理解的是，ZONE_DMA这个区域并不是说就专门为DMA工作，它只是内核为了管理某些特殊DMA访问
	的时候，划分的管理区域，实际上，它还能做其他内存访问的用途。
	DMA内存申请样例：

	void *dma_alloc_coherent(struct device *dev, size_t size, dma_addr_t *handle, gfp_tgfp);
	在ZONE_DMA区域时，gfp标志应该写为GFP_DMA。


	③  ZONE_NORMAL
		
		IA32下，内核为了让自己访问内存简单化，一开机就把一段物理地址，直接线性映射到3G
	以上的内核虚拟地址。这段内存的大小也有考究，假设物理内存超过1GB，那么内核的虚拟空间是无法
	全部一一映射物理地址的，因为内核虚拟空间只有1GB。那么怎么做呢？内核就把896MB以下的内存，
	称呼为低端内存low memory，包括zone_dma + zone_normal。低端内存，在开机的时候就做了线性映射，
	请注意，线性映射，不是意味着低端内存已经被内核用掉了，只是说，内核已经有了这个映射的页表，
	当内核真正需要用到内存的时候，还是需要申请的！
	low_memory内存申请API：
	
	物理地址与虚拟地址之间的直接映射API：
	phys_to_virt
	virt_to_phys
		

	动态申请内存API：
	kmalloc kfree 返回在物理内存连续的虚拟地址（线性地址），比如DMA操作
	vmalloc vfree 返回在物理内存不连续的虚拟地址，虚拟地址是连续的，明显用在high memory区域。


	④ HIGH_MEMORY
	内核想访问所有的内存，0~896在开机的时候做了线性映射，剩下的，内核只能通过高端内存的方式
	动态映射了。


	
	⑤ 这里枚举两个典型的体系下内核的空间分布：


	32位x86系统Linux内核的地址空间

							4GB————————————————————————保留去
							|——————————————————————————	
	|		|    high memory                |——————————————————————————专用页面映射区
	|		|				|——————————————————————————高端内存映射区
	|		|				|-------------------------vmalloc
	|		|				|
	|		|				|
	|———————————————|896MB				|	物理内存直接映射区域
	      normal					|
	|_______________|16MB low memory		|
	|______dma______|				|3GB



	32位ARM体系中Linux内核空间地址			| 向量表
							|—————————————
							|vmalloc
							|——————————————
							|
							|
							|
							|DMA+normal
							|——————————————
							|高端内存映射区
							|——————————————
							|内核模块



	这里资料有限，分析到非常浅显，只能初步总结：
	在不同体系结构中，进程中的内核虚拟地址空间的布局也不尽相同的！‘


	8，Linux buddy算法

		【核心思想：空闲页，按照2的n次方管理！】
		理论基础：任何正整数，都可以拆分为2的n次方之和。
		for example:
		15 = 2^3 + 2^2 +2^1 +2^0 = 8+4+2+1
		
		个人理解：
		首先，所有空闲的物理内存，都使用buddy算法进行管理。
		也就是页为基本单位，以页的2的n次方分组，每个组里面的成员用链表管理。
		当我们需要分配m页大小的内存块的时候，就会先找到包含m页大小的内存块，
		分配后剩余的再拆分放到其他大小的链表中。

		cat /proc/buddyinfo
		Node 0, zone      DMA      0      0      1      0      2      1      1      0      1      1      3 
		Node 0, zone    DMA32  46597  26232   8782   3807   1521    271    103     46     25      8      2 
		Node 0, zone   Normal    590   1884   3902   2322   1898    697    198     70     18      3      0 

					 |			|	
				表示2^0 = 1页的内存    表示2^3 = 8页的内存


		
		深入理解buddy分配器：
		
		① 数据结构：
		struct zone	|free_area[0]	————> page2^0	page2^0	page2^0
				|free_area[1]	————> page2^1	page2^1	page2^1
				|free_area[2]	————> page2^2	page2^2	page2^2
				|...
				|
				|free_area[n]	————> page2^n

			|		|			|
			v		v			v	
		管理一个zone分区     成员链表		成员包含物理页的数量
		也可以得知：
		系统有多少个zone，
		就有多少个独立的buddy内存分配器
		
		



		② 初始化
		Linux内核初始化过程中，bootmem/MEMBLOCK内存分配器初始化完物理内存之后，
		使用循环将空闲的物理内存按照一定的长度，实际就是通过buddy算法转移给buddy内存分配器！
		当分配完成，bootmem/MEMBLOCK分配器任务就完成了，内核就直接采用buddy内存分配器来
		管理可用的内存。
		

		
		③ buddy的定义，满足以下三个条件
			A 两个内存块大小相等
			B 两个内存块地址相邻
			C 两个内存块必须是在同一个大块的内存块中分离

			通俗的说，buddy的定义是这样的：
			两块内存，从同一个大块内存分离出来，他们大小相等，地址相邻，称为伙伴！


		④ buddy算法的分配原理
			前面知道，内核使用zone + free_area，并通过块的大小
			划分到2^n中，来管理每个区中空闲的物理内存。
			在分配的时候，首先在刚好匹配大小的链表中遍历查询是否有空闲块，
			如果没有，就往上一层找，找到就先分裂，对半分，一半插入上一级链表
			剩下一半，查询是否还需要分裂，如果还能分裂，继续分裂插入到上上一级
			链表，直到满足分配需求为止。

			for example：
			
			alloc_page(GFP_DMA, 4);//GFP是Get Free Page的缩写！
			就先到zone_dma的buddy内存管理器中遍历free_area[2]是否存在空闲块。
			如果没有，就到free_area[3]中查找，如果此刻找到了，就先分裂找到的空闲块，
			一半插入free_area[2]，一半用于分配需求。

		⑤ buddy算法的释放原理
			buddy分配算法的逆过程。
			当释放一个内存块的时候，先到对应的链表中查询，是否有伙伴存在，
			回顾一下，伙伴有啥？大小相等地址相邻，都是从一个大块中分离出来的。
			ok，如果找不到伙伴，就直接插入到当前链表的链表头。
			如果找到伙伴，就会链表中摘下伙伴，合并成一大块，插入到下一级链表。
			然后继续考察是否又存在伙伴。。。
			
		⑥ bitmap通过异或来映射伙伴分配与空闲
			这个过程，我们需要借助bitmap位图的帮助：
			bitmap中某一位，对应两个互为伙伴的块。
			bit = 1，表示其中某个伙伴分配出去了，
			bit = 0，表示两个伙伴块，都是空闲的
			
		
		⑦ 从上面我们可以知道，每个zone对应一个buddy分配器，那么各个分配器之间是否存在联系？
		答：存在分配联系的，如下：
			
			zone_dma	---> zone_dma else OOM
			zone_dma32	---> zone_dma32 ---> zone_dma else OOM
			zone_normal	---> zone_normal ---> zone_dma32 ---> zone_dma
			zone_highmem	---> zone_highmem ---> zone_normal --->zone_dma32--->zone_dma
			
			简单的说，首先会在当前分区的zone中查找是否有空闲分配，如果没有，就会
			按照上面的对应关系去其他区域分配，不管如何，如果没有足够的内存分配，
			系统都会OOM！

			OOM的理解，后续章节再回顾！
			


		⑧ 核心API(pages)：
			alloc_pages(GFR_ZONE, order);
			free_pages(unsigned long addr, order);





	9，CMA，连续内存分配器,解决buddy算法导致的内存碎片化问题

		实际解决例如DMA这样没有经过MMU的连续的大块物理内存的需求问题！
	
		前面有描述到，Kmalloc可以申请物理内存连续的虚拟地址也连续的内存，
		但是Kmalloc无法申请大块的内存地址！！！所以，这里Kmalloc无法帮上忙了。

		

		工作原理：
			预留一段内存给驱动使用，
			当驱动不用的时候，buddy可以分配给用户进程用作匿名内存或者页缓存。
			当驱动使用的时候，就将进程占用的内存回收或者迁移，拿回占用的预留内存。


		初始化：
			在buddy初始化之前，但又在memory block early allocator初始化之后。
			初始化接口：
				dma_contiguous_reserve(phys_addr_t limit);//limit是CMA上限

			
		分配：
			CMA并不直接开放给driver的开发者。开发者只需要在需要分配dma缓冲区的时候，
			调用dma相关函数就可以了，例如dma_alloc_coherent。
			最终dma相关的分配函数会到达cma的分配函数：dma_alloc_from_contiguous


		释放：
			释放的流程比较简单，同分配一样，释放CMA的接口直接给dma。
			比如，dma_free_coherent。它会最终调用到CMA中的释放接口：free_contig_range。
			直接遍历每一个物理页将其释放到buddy系统中。


		实践：
			我们可以在dts中，轻松配置一个特定的CMA区域用作某些驱动的特殊用途：
			  reserved-memory {
			 	 	#address-cells = <1>;
					#size-cells = <1>;
					ranges;
               			 /* global autoconfigured region for contiguous allocations */
				 linux,cma {
					compatible = "shared-dma-pool";
					reusable;
                        		size = <0x4000000>;
                        		alignment = <0x2000>;
                        		linux,cma-default;
		                };

		                display_reserved: framebuffer@78000000 {
		                        reg = <0x78000000 0x800000>;
                		};

 
 		               multimedia_reserved: multimedia@77000000 {
		                        compatible = "acme,multimedia-memory";
		                        reg = <0x77000000 0x4000000>;

              			};
        		};

		如果我们没有配置特定的cma区域，就会从"linux，cma-default"指定缺省的CMA池中获取。

		额外知识：
			在分配的时候，我们看到分配CMA的时候调用第一节的API为
			dma_alloc_coherent();
			到目前位置，我们都在讨论CMA是分配连续的物理内存块，但是
			它真的就一定是物理连续的吗？

			答案是不一定的，这取决于是否存在IOMMU（ARM中称为SMMU）
			SMMU的产生主要是为了解决虚拟化平台下的DMA重映射问题。
			

			CPU <-----MMU----> memory
					^
					|
				     IOMMU/SMMU
					|
					v
					外设/DMA

			DMA，外设和内存的连接件，用于解放CPU。外设可以通过DMA，将搜集的数据批量传输
			到内存，然后再发送一个中断通知CPU去内存取。这样减少了CPU被中断的次数，提高
			了系统的效率。DMA要能够正常工作，首先要进行正确的配置（包括通道选择、
			DMA源外设地址、DMA目的内存地址及尺寸等信息）；其次，一般需要连续的
			一段或多段物理内存。由于DMA不能像CPU一样通过MMU操作虚拟地址，
			所以DMA需要的是连续的物理地址。

			这样就有一个问题，对于一些虚拟化的系统而言，它是无法正常工作的。那么怎么办？
			现代计算机，引入IOMMU/SMMU的架构，对于非CPU的外设而言，也提供了一个MMU部件。
			如果想深入，可以参考帖子：
			https://blog.csdn.net/gaojy19881225/article/details/82585973
			https://blog.csdn.net/finicswang/article/details/96107339

			
			
			
















	
	

	





		












































			
