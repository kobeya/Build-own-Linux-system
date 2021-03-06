一、建立swap分区的模型：

				程序（正在执行部分 + 很长时间没有执行的部分）
								|	^
					硬盘I/O----------->     |	|运行的时候，再从swap恢复
								|	|触发page fault启动置换页
								V	|
					硬盘中一部分空间---> 【spwap分区】		
					|
					V
			所以，有时候启动一个很久没有启动的程序，你可能会听到硬盘“哒哒”的声音！	


	在系统物理内存不够的时候，将程序很长时间没有执行的部分，放到由硬盘中的一个交换分区。
	对服务器的性能至关重要，通过调整swap分区，有时可以越过系统性能瓶颈，节省系统升级费用。
	

	摘自百度百科的一些理解：
	
	A swap也是同物理内存一样，分页管理的！

	B swap空间是否有限制？答：有，以前swap限制为128MB，现在限制为2GB。
		128MB：旧版本的Linux，使用bitmap的方式来管理swap，也就是用swap空间的第一页的每一个
		bit来映射整个swap分区，（最后10bit是用来描述swap版本信息）。
			bit = 1，表示对应的页可用，
			bit = 0，表示对应的页存在坏区，不可用。
		这样做的优点是方便管理swap分区的坏块。
		但是，系统设计者后面认为：
		1，硬盘质量很好，坏块很少
		2，如果有很多坏块，那也不应该作为swap分区使用

		于是，现在的swap分区，取消了128MB的限制，直接用地址访问，限制2GB。

	C 分配太多的Swap空间会浪费磁盘空间，而Swap空间太少，则系统会发生错误。

	D 存在很多用户工具管理、监测、创建swap分区，比如可以使用swapon -s,查看当前swap的使用情况等。
		当然还有使用/proc,虚拟内核文件来监测。

	
其他帖子的额外收获：
	https://www.cnblogs.com/sunshine-long/p/11045030.html
	swap的设置：
		当物理内存使用完或者达到一定比例之后，我们可以使用swap做临时的内存使用。
		当物理内存和swap都被使用完那么就会出错，out of memory。对于使用多大比例内存之后
		开始使用swap，在系统的配置文件中可以通过调整参数进行修改。
		cat  /proc/sys/vm/swappiness 60
		该参数可以从0-100进行设置。0就是最大限度使用内存，尽量不使用swap；
		100就是积极使用swap。这个具体的通过系统的算法进行确定。
		那么swap大小如何确定？
		根据centos官网介绍可以得出如下公式：
		M = Amount of RAM in GB, and S = Amount of swap in GB, then If M < 2, S = M *2 Else S = M + 2。  
		而且其最小不应该小于32M(never less than 32 MB.)。

	swap是磁盘空间，内存操作是I/O，速度是很慢的。物理内存的操作的load/store。

		那么怎么优化这个swap的访问速度？
		答：采用多个swap轮流访问。
		如果有多个swap交换区，每个swap会有一定的优先级，该优先级也可以调整。
		swap空间的分配会以轮流的方式操作于所有的swap，这样会大大均衡IO的负载，
		加快swap交换的速度。


