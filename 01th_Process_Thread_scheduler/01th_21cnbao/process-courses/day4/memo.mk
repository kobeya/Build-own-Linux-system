主题：多核、cgroup、硬实时


一、负载均衡

	背景知识：
	
		A 在多核或超线程中，每个核都有一样的SCHEDFIFO/SCHED_RR/normal_CFS的调度算法，
			在Linux就像分布式系统一样，自动在各个核之间做分布式的负载均衡！

		B 每个核都有可能push task_struct到其他核，也有可能从其他核pull task_struct到自己。

		C 核，是以劳动为乐的，也就是不想自己闲着！

		D time 命令详解：
			比如 time ./a.out
			time ./a.out 
			main pid:23047, tid:140607226689344
			thread pid:23047, tid:140607209899776
			thread pid:23047, tid:140607218292480
			^C （这个地方，我CTRL+C用信号终止了进程，然后在进程结束的时候，自当打印如下）
				
			real	0m1.703s //程序实际执行时间，包括执行+阻塞用时
			user	0m3.400s //系统执行的时间和(两个线程的时间和)
			sys	0m0.000s //内核态时间
									user+sys
			通过这三个值，可以计算，这个进程的CPU利用率：—————————————— X 100%
									  real
			其实，用top最简单了，直接就可以获取到CPU利用率。

			再扩展：
			type -a time //使用type命令查看time的信息
			time is a shell keyword
			time is /usr/bin/time
			在使用如下命令获取更加相信的进程信息！
			/usr/bin/time -v ./a.out 
			main pid:27035, tid:140633758570304
			thread pid:27035, tid:140633750173440
			thread pid:27035, tid:140633741780736
			^CCommand terminated by signal 2
				Command being timed: "./a.out"
				User time (seconds): 8.55
				System time (seconds): 0.00
				Percent of CPU this job got: 199%
				Elapsed (wall clock) time (h:mm:ss or m:ss): 0:04.27
				Average shared text size (kbytes): 0
				Average unshared data size (kbytes): 0
				Average stack size (kbytes): 0
				Average total size (kbytes): 0
				Maximum resident set size (kbytes): 1780
				Average resident set size (kbytes): 0
				Major (requiring I/O) page faults: 0//磁盘中读取页的数量
				Minor (reclaiming a frame) page faults: 75//缓存页数量
				Voluntary context switches: 2
				Involuntary context switches: 52
				Swaps: 0 //进程被交换出内存的次数
				File system inputs: 0
				File system outputs: 0 //文件系统读写的数据量
				Socket messages sent: 0
				Socket messages received: 0
				Signals delivered: 0
				Page size (bytes): 4096 //页的大小（字节）
				Exit status: 0

		E 为啥现在的多核CPU没有单数？
			其实原理很简单，主要是为了排版以及PCB的走线连接针脚的问题，
			比如偶数可以拍成两排，容易布置核心，并且还涉及到了核心之间的数据交换，
			偶数的设计会更加的杰出优秀，因为单列中间如果有一个核心的话，会很影响工作效率		
			然后其实我们市面上看到的核是厂商给我们看到的样子，也许他们还做了核屏蔽。

		F 命令sh -c的解析：https://www.cnblogs.com/ggzhangxiaochao/p/10773694.html
			简单总结：在shell中断，比如terminal中执行一条命令：
			sudo echo "123" >> file（file是root权限）
			此时，还是会报权限的错误，因为sudo只修饰第一个命令echo,
			第二个命令>>还是没有权限的，现在，sh -c可以做到这个事情：
			sudo sh -c 'echo 124 >> file'
		

		G Linux有调度抖动，时间为delta。
			


		H 这里注意一个锁的问题，之前我们可以看到有spinlock,metux
			首先，metux、spinlock都是在多核里面使用的
			如果单核，持有任何锁，都已经把当前的核关闭抢占了，也就没意义了。
			metux，在多核里面，某个核持有锁，那么其他核上的进程，就睡眠。
			spinlock，在多核里，某个核持有锁，那么其他核上的进程，就忙等待。



		I 当一个patch merge到内核主线，那么就不是补丁，已经是Linux内核的内容了。

		J 一个进程是否硬实时，是我们程序员决定的，我们可以使用一些接口或者工具去实现！


	1，RT进程

		A N个优先级最高的RT进程，自动均衡分布在N个核，这样RT进程就可以及时抢占CPU，
			RT更加强调的是实时。
		B 
	
	pull_rt_task()
	push_rt_task()


	2，normal进程

	周期性负载均衡，操作系统节拍来了，就会查询自己核与其他核的忙闲程度是否达到某个差值
	IDLE时负载均衡，一个核闲下来，准备跑0，就会查询其他核，会pull过来跑
	fork和exec时负载均衡，fork，会把task_struct推到最闲的核上


	所以，Linux中的task_struct，也就是线程，可能会在一段时间内，在多个核中执行。
	
	问题：那么我们程序员可以控制线程在具体某个核上运行吗？
	答案：可以。
		|
		|
		|
		v
	3，设置affinity(亲和力)
	
	有以下接口，通过设置对应的掩码，来指定进程对某个CPU的亲和力：
	int pthread_attr_setaffinity_np(pthread_attr_t *, size_t, const cpu_set_t *);
	int pthread_attr_getaffinity_np(pthread_attr_t *, size_t, cpu_set_t *);
	int sched_setaffinity(pid_t pid, unsigned int cpusetsize, cpu_set_t *mask);
	int sched_getaffinity(pid_t pid, unsigned int cpusetsize, cpu_set_t *mask);
	比如：

	口口口口口口

	现在有6个核，我想设置进程对核心1、核心2具有亲和力
	只要设置cpu_set_t的值为110、100
	对应的掩码是0x6,0x4

	除了上述的API，还有工具来实现：
	taskset
	比如：
	taskset -a -p 01 21451
	-a, all
	-p, core, 01表示第一个CPU，也就是核0，掩码！

	这个工具可以实现，将某个进程，设置在指定的CPU上均衡运行。

	4，中断IRQ affinity，中断的负载均衡
	分配IRQ到某个CPU：
	echo 01 > /proc/145/smp_affinity //每个中断号下面都有一个smp_affinity

	但是中断也有个问题，就是当某个中断在某个核上又不断的产生软中断的时候，这些软中断是继承
	中断的affinity的，也只在这个CPU上跑，这样就可能导致，一个核很忙，其他核很闲的情况发生。
	比如，我们的网卡，在收发包的时候，该核的中断负载很大，TCP/IP协议栈的处理在软中断，软中断
	的负载也很大。

	解决办法：
	有一个patch可以解决，RPS的补丁。
	这个RPS补丁，可以做到软中断的负载均衡。
	/sys/class/net/eth1/queues/rx-0/rps_cpus,这个rps_cpus可以设置多核软中断的负载均衡。

	
二、 cgroup（进程的分群）

	
	1，定义不同的cgroup CPU分享的share

	2，定义某个cgroup在某个周期里面最多跑多久

	按照宋大侠的例子，初步认识一下cgroup的用法：
	首先，在没有使用cgroup来管理进程的时候，进程是均衡分布在各个核的。
	然后，我们可以通过cgroup来管理进程，将多个进程进行分群。
	在路径/sys/fs/cgroup/cpu/下，创建cgroup群组！
	比如：mkdir A/B (删除使用rmdir A/B，删除创建的目录就可以)
	然后进到A或者B目录，看到很多自动生产的文件，此时，我们可以把进程，
	通过sudo sh -c 'echo pid > cgroup.procs',加入到对应的群组，此刻，
	进程只能在该群组的核心上运行！

	我们还可以设置群组的权重来改变群组获得CPU的资源比例：
	sudo sh -c 'echo 2048 > cpu.shares'

	设置在群组中，进程的获取时间比例：
	sudo sh -c 'echo 20000 > cpu.cfs_quota_us'


	3，Android和Cgroup

	Shares:
		apps, cpu.shares = 1024
		bg_non_interactive, cpu.shares = 52

	Quota:
		apps->	cpu.rt_period_us: 1000000 cpu.rt_runtime_us:80000
		bg_non_interactive-> cpu.rt_period_us:1000000 cpu.rt_runtime_us:700000
	


	4，Docker和Cgroup

	$ docker run --cpu-quota 25000 --cpu-period 1000 --cpu-shares 30 linuxep/lepvo.1
	在docker启动的时候也可以设置cgroup
	然后就会在cgroup下，自动创建对应的id号的cgroup管理器。
	这样，我们就可以使用cgroup来管理各个容器对资源的比例。
	（后续找时间整理docker这个工具的用法。）








三、Hard realtime 硬实时
	1，硬时间，不是越快越好的意思，其实最重要的意思是：可预期！
		比如，我们唤醒一个硬实时的进程，这个进程，在硬实时的条件下，
		唤醒时间是可以预期的，也就是说，唤醒的这个过程，是在一个限制的时间范围内！
		不会超出某个限制。

		example:
			发射火箭，要求按键触发，到发射，这个时间间隔1ms
			那么只要在1ms内能发射，就符合硬实时的要求；
			如果，超出1ms发射，那么就会引发灾难性的后果！

		Linux，不是一个硬实时系统，

		example：
			播放视频，要求每一帧的时间间隔是20ms
			如果时间间隔超出了20ms，也不会引发灾难性的后果。

		总结：硬实时，实际是程序的需求，是根据程序本身的需求来划分的。



	2，Linux实时性能测试工具cyclictest
		一般主要用来测试使用内核的延迟，从而判断内核的实时性。

		安装：sudo apt install rt-tests

		sudo cyclictest -p 99 -t1 -n
		# /dev/cpu_dma_latency set to 0us
		policy: fifo: loadavg: 0.55 0.66 0.82 1/951 30415          

		T: 0 (30361) P:99 I:1000 C:  22555 Min:      1 Act:    1 Avg:    2 Max:     402

		T:0		序号为0的线程
		P：99		线程的内核优先级为0
		C：22555	线程的时间间隔计数
		I：1000		时间间隔为1000us
		Min:1		最小延时1us
		Act:		最近一次的延时
		Avg：		平均延时
		Max：		最大延时，注意这个值，是随系统负载而变化的！这个值也是最重要的！


	3，为什么Linux不是一个硬实时的操作系统？
		A Linux随着版本的更新，从2.0到2.6版本，Linux支持的抢占越来越多，到2.6版本，
			内核支持抢占。当然还有不少区域是不支持抢占的，不过我们有patch能做到
			更全面的支持，也就是Linux能更加接近硬实时的条件。
	


		B Linux跑起来，CPU的时间都花在4类区间中：
			① 中断 （2.6.32版本之后，中断是不能嵌套的）
				在早期，我们注册一个中断的时候，会申请一个flags-IRQF_DISABLED，
				也就是在执行中断处理的时候，禁止本核其他所有的中断，后期，这个
				flags干脆直接变成了申请注册中断必要的特性，也就是在中断处理时，
				会屏蔽其他所有的中断响应！
				这里思考？如果在中断处理，又来一个中断怎么办？
				答：首先，我们的中断，都现在中断控制器的队列中，
				当某个中断在处理时，其他中断会被终端控制器pending。
				当中断处理了，终端控制器才会把中断发给CPU处理！
				中断控制器是没有上限，但是有可能丢中断。
			
			② 软中断 （软中断可以嵌套中断，但是不能调度,软中断是一个比进程上下文
				优先级更高的上下文，所有的进程，都无法抢占软中断上下文！）
			③ 持有锁不可调度的进程上下文
			④ 可调度的进程上下文	
		
			明显，我们只有可调度的进程上下文可以调度！
			在① ② ③ 区间，只能等待返回，才能参与抢占！

			结论：硬实时抢占不稳定，所以Linux不是一个硬实时的操作系统！


		举个例子：


					触发一个RT进程         	   RT抢占		RT运行

		 RT task		    |～～～～～～～～～～～～|————————————————————|



								释放锁	
		Normal task  ————|			|————————————|

				   这个时候，拿到锁
		syscall		  ————|	

				         一个中断	
		IRQ1 handler	       ——————|

						又是一个软中断
		IRQ2 handler		     ————————————|

				t1   t2      t3		 t4	      t5	
			time	|————|———————|———————————|————————————|———————————————————


		上述，我们可以看到，RT很早就触发了，但是normal进程分布经历持有锁、中断、软中断
		三个区间，进程都无法抢占，这个三个区间的耗时，我们无法得知，也无法确定，所以
		RT的抢占时间，是不确定的，这就导致Linux无法实现硬实时！


	
	4，可以让Linux变成一个硬实时的操作系统吗？

		答：社区已经有补丁可以做这个事情，这个补丁的内容包括：
			A spinlock迁移为可调度的mutex
			B 实现优先级继承优化--->注意，这个Linux内核已经实现了，也就是现在它就是Linux
				内核的一个功能了，不需要额外的补丁来产出！
			C 中断线程化，把中断放在一个线程里面去做，这样，这个线程就可以被其他线程抢占
			D 软中断线程化，一样的道理
				中断线程化，中断就变得非常的短，只有一个话做返回，剩下的留在线程
				里面做。

			这样，我们就把① ② ③ 区间也变成了④ 区间，也就是可以被抢占！！！
			这样，就可以满足硬实时抢占的可预期性！！！

		
		注意：一个硬实时操作系统，也只能保证一些进程可以硬实时，并非所有的进程都需要或者
		都是硬实时！！！	


		
