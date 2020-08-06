#摘要
	slab、/proc/slabinfo和slabtop
	用户空间malloc/free、内核空间kmalloc/kfee与Buddy的关系
	mallopt
	vmalloc
	Linux为应用程序分配内存的lazy行为
	内存耗尽（OOM）、oom_score和oom_adj
	Android进程生命周期与OOM





正文


1，slab、/proc/slabinfo和slabtop

	slab分配器的缘由：

		—— 外部碎片：有空闲的内存，但又无法满足分配需求（比如剩余的内存太小，内存不连续）
	碎片 |
		—— 内部碎片：分配出去，但是又无法使用的内存

	buddy算法分配的内存，虽然已经有一定程度的解决外部碎片的问题，
	都是以页为单位，最小为4kb(体系相关)，但是进程有时候需要申请更小粒度
	的内存，这就需要更小粒度的内存分配器来满足我们的需求。

	因此，借鉴Sun公司的Solaris系统的slab内存分配算法：
	基于buddy算法的内存空间对小块内存进行管理。

	在理解slab内存分配器之前，必须知道的知识点：

	① 我们在描述slab的时候，使用了高速缓存的概念，请注意，这里使用的高速缓存，并不是硬件
	上的高速缓存，而且系统为访问内存中的某些数据类型，初始化这些对象的时间远超过
	创建撤销某些对象的时间，所以，干脆预先基于buddy
	算法，在内存本身上开辟的内存空间，这是软件上实现的管理手段，只不过它的效果可以达到
	类似硬件高速缓存的快速命中的目的！
	当然，实际中，还有一点就是slab对象数据结构会自行对齐，符合硬件高速缓存和TLB的支持。

	② slab slub slob简述：
	slab是基础，slub是基于slab的改进，满足大型机器的需求。slob也是基于slab，满足嵌入式
	小型机器的需求。

	【Linux内核开发提供各种机制，我们程序员，按照选择自己的策略！】
	

	③ 总结上述，得出slab内存分配器的优势：
		A 优化buddy算法大块内存分配导致的内部碎片问题
		B 优化频繁分配对象时初始化、创建、撤销的时间开销
		C 通过着色技术，调整对象以更好的使用硬件高速缓存（！）


	slab分配器的结构：
				

	|------------------------------>为了更加提高效率，slab分配器中，还为每个CPU提供
	|				struct array_cache __percpu *cpu_cache;
	|				当CPU需要申请某个对象的内存空间的时候，会先检查
	|				这个数据结构中是否有空闲的对象，没有才会到slab分配
	|				器的缓存链表中遍历查找！
	|				
	|	Cache_chain 缓存链表
	|		|每个元素都是一个缓存结构体			slab是可以移动的，
	|		|						随着slab中的对象分配情况移动
	|		V						到不同的slabs队列中！
	| 同类型对象--->Kmem_cache					    ^	
	|		|				slabs_full>>>       |
	|		V	    包含一个slabs队列			 ->slab ->page(object...)
	-------------->	Kmem_cache ------------------>	slabs_partial--->->slab ->page(object...)
			|						 ->slab ->page(object...)
			V				slabs_empty>>>                   |
			Kmem_cache							 |
			|								 |
			V								 V	
		API:kmem_cache_create	                               API: Kmem_cache_alloc获取对象
		    Kmem_cache_destory					    kmem_cache_free
		    (只能销毁空的kmem_cache!)						|
			|								V
			V					获取object对象的流程大致描述：
	Kmem_cache实际还细分为两种类型：               	        首先，先查找cpu_cache的缓存池
	A 普通							是否有空闲对象，如果没有，
	B 专用							那么就到kmem_cache_node中查找，
	|							如果还是没有，
	|							就利用buddy算法获取新的页框，
	|							然后从新的页框获取空闲对象！	
	V
	A 普通kmem_cache，在系统初始化期间调用kmem_cache_init建立，用于SLAB分配器本身的目的。
			
	B 专用kmem_cache，就是我们接触到的slab高速缓存对象池了。API就是上面描述的API。





	slab分配着色
		缘由：相同大小的对象，倾向于存放在硬件高速缓存内相同偏移量的地方，目前我个人理解
		为，本身相同大小的对象，都在一个kmem_cache的slab缓存池中，根据空间局部性原理，
		这个缓存池内部的对象，在其中一个成员缓存在硬件高速缓存，其他靠近的成员也应该会被
		缓存在硬件高速缓存中。
		这里就出现一个问题，在不同的slab中，具有相同的偏移量很可能在同一个高速缓存中。
		
		slab coloring:
			给各个slab增加不同的偏移量，设置偏移量的过程，就是着色！
			这样，就可以让不同对象映射不同的硬件cache line中。
			



2，slub内存分配器
	缘由：它是基于slab分配器发展而来。在slab分配器中，使用了大量的缓存队列等复杂的层次结构，
	空间换取时间，为了优化大量的空间问题，引出了slub内存分配器。

	性能：
	据内核开发人员称，SLUB相对于SLAB有5%-10%的性能提升和减少50%的内存占用
	（是内核对象缓存占用的，不是全局哦，否则Linux Kernel可以修改主版本号了）。
	所以SLUB是一个时间和空间上均有改善的算法，而且SLUB完全兼容SLAB的接口，
	所以内核其他模块不需要修改即 可从SLUB的高性能中受益。
	SLUB在2.6.22内核中理所当然的替代了SLAB。（2.6.24才完全替代，也就是说，我们现在接触到的是
	SLUB内存分配器）

	接口基本等同与slab

	原理：
	
	引用这个帖子理解：https://blog.csdn.net/lukuen/article/details/6935068

	（1）内存分组：96B、192B、2^3、2^4...2^11,共11组，再大就干脆使用buddy的接口分配内存了






	96B

	192B

	2^2B-----------> kmem_cache                                    ----->
			 ...                                           -----> n个slab单元 
	2^3B		 node -----------------------> Kmem_cache_node ----->
			 ...                             partial      
	2^4B		 cpu_slab -> kmem_cache_cpu      ...
                                      ...                full
	...			     freelist--|          ...
                                      ... 保存下一个空闲     
	2^11B	                     page    object的地址
	                                |            
	|                               V
	|	                  只有一个slab单元---------> {object...}
	|	                  （slab可能包含1+页）
	V
	kmalloc_cache[12]
	(我觉得这里应该是11？)
	struct kmem_cache kmalloc_cache[PAGE_SHIFT];
	这样看，每个组的成员，
	就像是一个只出售某个大小固定的内存块的
	零售商！SLUB总共有12个这样的零售商。



	分配过程：

	配图看帖子吧，我这里给出文字描述。
	A kmem_cache创建出来的时候，只有空的kmem_cache_node + kmem_cache_cpu，也就是没有任何slab
	B kmem_cache_cpu首先向buddy中，获取空闲的页，并分成多个object
	C 取出其中一个object，标志为已被占用，返回给用户，其他标志为空闲，留在kmem_cache_cpu中
	D kmem_cache_cpu中的freelist指向下一个空闲的保存在kmem_cache_cpu中的object
	E 最简单的情况下，获取object的时候，可以直接在对应分组的kmem_ccache_cpu中取得空闲的object
	F 如果kmem_cache_cpu中已经没有空闲的object了，就去查询kmem_cache_node中空闲的object，
		首先查询的是partial链表。
		获取到有空闲的object的slab，并把一个空闲的object返回给用户。
	G 而且管理SLUB里面的内存分布：
		将kmem_cache_cpu中object全部标志为已被使用的slab放到kmem_cache_node中的full链表
		从kmem_cache_node中的partial链表中获取不满的slab放到kmem_cache_cpu中。
	H 最后，如果kmem_cache_cpu/node中都没有空闲的object了，也就是全部都是满的slab，那么，
		向buddy系统申请新的slab内存页，并挂在kmem_cache_cpu中。

	释放过程：
	
	核心思想，释放的object放回它出生的slab中即可
	细分各种情型如下：
		A 如果放回去的slab本身是partial的，那直接插入即可
		B 如果放回去的slab本身full的，插回去变成partial，那就将该slab迁移到partial链表
		C 如果放回去的slab本身是空，插入后，直接释放掉slab，也就是完全空闲的slab，需要释放掉


		



3，slob内存管理器
	simple list of blocks

	产生缘由：
	上面的还是无法最好的满足小型的，嵌入式系统，引入slob分配器。

	
	组织结构：
		


		总共三条不满的空闲链表。
		永远都是占用一个页框的大小？

				 ----> free_slab_large,分配小于4K的内存块

		partial free list----> free_slab_medium,分配小于1024Byte的内存

				 ----> free_slab_small,分配小于256Byte的内存块



		如果，slob需要分配大于1页的内存块，那么直接向buddy系统获取，返回内存块的第一页的
		首地址，并需要修改首页的flags字段为private。

		Slob分配器将描述slob的变量打包成一个结构，然后
		和页描述符struct page一起组成一个联合体，
		这样就可以直接利用页描述符已占有的空间，
		将页描述符中无关紧要的字段填充为slob的有效描述字段，
		这样便可以省下一笔内存了！
	

	分配原理：

		kmem_cache_alloc_node()函数用来分配一个专用缓存的对象。
		A 由于slob为PAGE_SIZE大小，因此首先要判断要求分配的对象的大小是否在这个范围内，
			如果是，则通过Slob分配器来分配，否则的话通过伙伴系统分配

		B 然后，根据需要分配的内存大小选择从哪个全局不满的空闲链表中查找slob，遍历它
		C 如果找不到，向buddy获取新的page，然后设置page的属性，再进行分配。
		D 




4，工具与窗口： cat /proc/slabinfo && sudo slabtop


	cat /proc/slabinfo，描述系统中目前的slabinfo分布细节
	sudo slabtop，描述当前系统实时的slab使用情况


5，mallopt




