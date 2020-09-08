【导读】
	01，camera
		1）camera Android框架、内核启动流程
		2）camera的bring up经验
		3）camera的一些调试经验

	02，display
		因为我们现在的屏基本使用mipi传输数据，而且其实bsp部分目前对我来说没有多少挑战，
		填充driver的一些索引，这里就不展开了，直接软连接到mipi

















编写一个code demo
从bsp driver 到 JNI 到framwork再到app的实例。
一、驱动的三种写法

	有如下三种方法：
	①传统方法：在代码led_drv.c中写死；
	②总线设备驱动模型：
	　a. 在led_drv.c里分配、注册、入口、出口等
	　b. 在led_dev.c里指定引脚
	③使用设备树指定引脚
	　a. 在led_drv.c里分配、注册、入口、出口等
	　b. 在*.dts里指定引脚

	可以看到，无论何种方法，驱动写法的核心不变，差别在于如何指定硬件资源。




二、传统写法，dts

【一、准备阶段】
	1，写驱动的硬件或者芯片信息
		A，看电路原理图，获取引脚信息
		B，看芯片手册，获取引脚的控制信息
	






