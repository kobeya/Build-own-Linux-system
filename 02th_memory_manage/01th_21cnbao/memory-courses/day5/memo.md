
一、主题
	DMA和cache一致性
	MemoryCGroup
	脏页写回
	水位控制：内存何时开始回收
	swappiness大小的设定需要根据具体情况
	getdelays调试工具
	vmstat（分析硬盘和swap压力的工具）


二、关于DMA ZONE和dma_alloc_coherent
	1，DMA ZONE的产生原因：
	在X86 32bit的计算机中，ISA实际只可以访问16MB以下的内存，也就是说，不一定所有的DMA都可以
	访问到所有的内存，这是本质上硬件的设计限制。

	也就是说，在32bit的X86上，高于16MB的内存，DMA是无法访问，那么如果，确定DMA一定可以访问
	干脆做一个内存管理上的限制，把16MB的内存，单独作为DMA可以访问来管理。
	后续在访问的时候，通过标志字段GFP_DMA来访问即可。

	2，现在绝大部分的SOC都很厉害了，似乎DMA没有什么缺陷，根本不需要我们去指定DMA_ZONE。
	如果问，现在的DMA有多大，最好是看下SOC的芯片手册。

	3，DMA ZONE是专用的吗？
	答：不是，DMA_ZONE可以用于任何其他地方，只是说在我们DMA访问的时候，在这个区域申请而已。

	4，dma_alloc_coherent申请的内存来自DMA ZONE吗？coherent连贯的
	答：取决与硬件是谁，因为在一些地方，可以覆盖整个4G
	
	5，dma_alloc_coherent()申请的内存一定是物理连续的吗？
	答：如果IOMMU存在（ARM里面叫SMMU）的话，DMA完全可以访问非连续的内存，
	并且把物理上不连续的内存，用IOMMU进行重新映射为I/O virtual address (IOVA)。

	
三、DMA与cache一致性
	1，为什么出现DMA与cache一致性的问题？
	答：首先很多时候，DMA访问的时候，没有smmu，iommu，也就是说，dma访问不到MMU，也访问不到
	cache，此时，如果cache里面存在有dma与内存做数据交互的那部分数据，那么就很可能cache里面
	的数据与dma内存交换的数据，不一致！

	内存已经与DMA数据交互了，但是cache还保存原来的数据，如果cpu去读内存的数据，实际会读取
	cache里面旧的数据。

	甚至cpu去写数据的时候，使用回写策略，cache里面的新数据还没有写回内存，此时
	访问的内存一样得到老数据。

	2，那么怎么解决DMA与cache一致性的问题？
	答：最简单的办法，就是CPU访问DMA buffer的时候，不带cache。
	硬件上实现，不带硬件cache同步单元。

	但是这不是我们最想要的，我们还可以通过其他办法来实现
	有两类APIs：
	总的来说，有接口实现的，但需要SOC的厂商来提供底层支持。

					|低端内存，开机即已经映射
				|CMA---
		---申请		|	|高端内存vmalloc分配连续的物理地址，注意vmalloc本身分配
				|	不一定物理地址连续，但在这里，必须强调用在连续上。
				|
				|smmu iommu支持的DMA分配，外设的mmu


				|在分配内存的时候，设置页表的cache属性位uncache
				|
	DMA--- ---确保一致性方法|硬件实现支持，我们就当正常使用即可
				|
				|在代码中实现支持，此时内存分配好了，无法设置页表cache属性
				 在访问这块内存前，先回写cache，然后禁止CPU访问这块内存
				 再做DMA操作，直到完成。

		
		---API DMA_alloc_coherent









四、memory cgroup
	在/sys/fs/cgroup/memory目录下创建子目录，子目录里面就会自动生成配置文件，我们可以设置
	这些配置文件来空间内存的消耗与回收。
	dmesg可以看到具体的信息。

五、脏页写回（cache对内存，内存对磁盘，都是一样的）

	1，为什么要脏页写回？
	答：首先cache的空间有限，脏页指的是有数据的页，那么Linux中的脏页不能太多，否则就会占据
	太多的cache空间，也不能存太久，否则，突然断电，脏页的数据就丢失了。
	这样即可以有效使用cache，也能保持内存与cache中数据的一致性，包括内存也回写磁盘。

	2，那么怎么处理？
	答：Linux有一个机制可以控制脏页写回。
	可以从时间、空间两个角度去思考
	① 时间，通过API-dirty_expire_centisecs
	只要脏页的时间长度达到dirty_expire_centisecs，Linux有一个后台现场kernel flusher会写回。

	② 空间，当脏页在内存中的比例达到一定的程度，dirty_background_ratio一旦达到，
	后台的线程就开始写回脏页，请注意，这个是时候可能应用程序还在不断的写脏页，也就是可能
	回收脏页比不上新曾脏页，一直达到dirty_ratio，此时，前台的进程就会直接阻塞，必须等待
	脏页回写。
	也就是说，整个脏页回写的数量控制在dirty_background_ratio和dirty_ratio之间，进程不会阻塞。

六、水位控制
	
	Linux使用/proc/sys/vm/min_free_kbytes文件来控制水位。
	用水平来描述空闲内存的剩余量：
	可以设置/proc/sys/vm/min_free_kbytes文件来控制水平的配置值
	|
	|--------->high = min×6/4
	|
	|--------->low  = min×5/4，此时Linux已经有后台线程进行内存回收
	|
	|--------->min           ，此时，已经阻塞应用程序，并且在进程上下文回收内存direct reclaim





















