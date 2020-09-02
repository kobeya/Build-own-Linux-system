#if 0
一、本章主题	Linux为什么发生soft lockup

二、可参考文章：
https://blog.csdn.net/21cnbao/article/details/108250786
https://www.jianshu.com/p/e03b933fa2fe
https://blog.csdn.net/sunny05296/article/details/82858071
https://www.cnblogs.com/vedic/p/11194839.html
https://www.cnblogs.com/arnoldlu/p/10338850.html
https://blog.51cto.com/guowang327/1962739


三、BUG描述

	1，log参考
		1）BUG: soft lockup - CPU#3 stuck for 23s! [kworker/3:0:32]
		2）[ 56.032356] NMI watchdog: BUG: soft lockup - CPU#0 stuck for 23s! [cat:153]
		3）BUG: soft lockup - CPU#2 stuck for 67s
		4）kernel:BUG: soft lockup - CPU#0 stuck for 38s! [kworker/0:1:25758]
		5）kernel:BUG: soft lockup - CPU#7 stuck for 36s! [java:16182]
		等等
		从log可以看出以下三点：
		1）bug是的原因是因为触发soft lockup
		2）某个进程占用CPU时间值，比如23s、38s、36s等
		3）这个导致bug的进程的名称，比如cat、java等
		4）导致这个bug进程的pid，如 153、16182等



	2，这个是什么bug，有什么特性
		答：通俗点理解，就是当前CPU被某个进程占满了，长时间响应不了。
		也就是（默认）20s内，该CPU没有发生调度切换就可能抛出soft lockup。

		1）soft lockup是针对单独CPU而不是整个系统
		2）soft lockup指的是发生的CPU上在20秒(默认)中没有发生调度切换

	3，为什么会出现？
		答：从2的情况下，我们也可以知道答案，就是某个CPU，20s内都不发生进程切换，就会触发
			soft lockup！

		详细代码在kernel/watchdog.c中的逻辑
		大概如下：
		1）Linux内核会为每个CPU启动一个优先级最高的FIFO实时内核线程watchdogd
			kobey@pc:~$ ps -e | grep watchdog
			  145 ?        00:00:00 watchdogd

		2）这些内核线程，默认每隔4s就会喂一次狗，然后在喂狗的时候，刷新一个hrtimer的
			2倍的watchdog_thresh的值，这个值我们可以设置，默认是10，这也可以看到
			soft lockup涉及到一个20s的默认值
			kobey@pc:~$ cat /proc/sys/kernel/watchdog_thresh 
			10

		3）如果kernel.watchdog_thresh = 20后，依然没有发生喂狗动作，也就是没有发生
			进程调度，那么就会打印soft lockup的log


	4，出现这个bug后除了打印log外还有其他表征吗？
		通常，这个bug不会导致内核崩溃，但是如果我们设置它会触发panic，那么当然可以。
		所以，这个要根据实际情况而论。


	5，解决办法

		1）大部分情况可能因为死锁或者某个死循环导致，那么就按照这两个角度去分析
			比如，某个循环退出条件bug，自旋锁嵌套等等
			服务器行业的bug分析：
			https://blog.csdn.net/sunny05296/article/details/82858071
		2）使能panic
			/proc/sys/kernel/panic


	6，扩展hard lockup
		与soft lockup类似，用于维护一个长时间没有交互的硬件的机制
		如果有个CPU超过一定的时间，没有响应外部中断信号，也就是外设发生中断，好久
		没有得到处理，内核就会触发hard lockup

#endif
