/*
 *	本章解剖一些panic（oops）的触发、行为以及实现原理
 *	代码这里就不加进来了，在内核SDK中kernel/panic.c，这里仅文字阐述
 * */

一，认识panic
	panic是英文中是惊慌的意思，Linux Kernel panic正如其名，linux kernel不知道如何走了，
	它会尽可能把它此时能获取的全部信息都打印出来。

	1，有两种主要类型kernel panic，后面会对这两类panic做详细说明：

	1.hard panic(也就是Aieee信息输出)
	2.soft panic (也就是Oops信息输出)

	2，常见Linux Kernel Panic报错内容：

	(1) Kernel panic-not syncing fatal exception in interrupt
	(2) kernel panic – not syncing: Attempted to kill the idle task!
	(3) kernel panic – not syncing: killing interrupt handler!
	(4) Kernel Panic – not syncing：Attempted to kill init !
	https://www.xuebuyuan.com/1105362.html
	panic相对来说已经是比较高层次的接口，内核开发希望我们能分辨出哪些行为是有危险的，
	当出错的时候显式的调用panic让系统产生惶恐。

二，hard panic，现象很重，常见是各种死机

	1，出现hard panic的现象
		1）机器直接锁死，完全不能用
		2）打开终端，看到有内核dump信息出来

	2，出现的原因分析
		对于hard panic而言，最大的可能性是驱动模块的中断处理(interrupt handler)导致的，
		一般是因为驱动模块在中断处理程序中访问一个空指针(null pointre)。
		一旦发生这种情况，驱动模块就无法处理新的中断请求，最终导致系统崩溃。

	3，信息收集
		核心思路：根据panic发生的环境去尝试收集各种log
		1）如果开机后才发生panic，可以尝试获取dmesg、bugreport、logcat、串口等log信息
		2）如果没有完全开机就已经发生panic，那么可以尝试使用串口抓取log
			甚至可以尝试添加KDB工具在内核中，它的原理是将内核的加载放到一个shell环境中
			这样就可以打印出内核启动的log了，这需要一些工作。

三、soft panic

	1，出现soft panic的现象
		1）段错误导致crash
		2）在log中看到oops提示
		3）机器勉强还在跑，但不准时会crash

	2，原因分析
		凡是非中断处理引发的模块崩溃都将导致soft panic。
		在这种情况下，驱动本身会崩溃，但是还不至于让系统出现致命性失败，
		因为它没有锁定中断处理例程。导致hard panic的原因同样对soft panic也有用（
		比如在运行时访问一个空指针)

	3，还是抓取log，然后安装ksymoops工具来将内核符号信息转成有意义的数据来分析。

	总的来说，如果我们的产品出现panic，首先，可以根据当前发生panic的设备状态、设备版本差异、
	设备执行操作等信息初步分析可能发生panic的原因所在，这样就可以针对性去抓取有用的log信息
	方便再次复现的时候能捕获到直接有效的log，然后根据log去分析产生panic的根本原因所在。

	但是，这里要注意一个点，并不是panic的log就能帮助我们找到原因所在，因为一个问题的出现，
	可能会导致各种各种的panic，这个时候你单纯根据panic的log去分析也许无法解决问题，那么就要
	注意整个系统的现象，比如在出现panic之前的log是否存在异常。


四、分析panic log

	这个要根据具体来描述，采用不同策略，一般的处理方法如下
	1）如果能从log中看到某个地址有异常，我们可以使用add2line工具将地址所在的函数配合vmlinux打印出来
	2）我们知道，这些log的表现实际是体现出内核调用栈，我们可以使用objdump来反汇编，然后根据panic
	的信息找到对应的汇编代码，对照C代码的上下文来分析汇编的上下文确定C代码出现panic的位置。

https://blog.csdn.net/pansaky/article/details/90440356

