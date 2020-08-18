主题：设备树 Device Tree

2020年08月18日09:35:31 kobey

一、什么是设备树dts？
	1，设备树的由来？
	首先是Linus本人觉得受不了内核存在大量的关于硬件而无关内核关键的垃圾代码，引起了社区的轰动。
	然后就从其他工作站(SPARC)引入到Linux内核。
	在总线设备驱动模型的基础上，修改平台设备的构建，适配DTS。
	我们可以看下现在的模型：
	dts ---dtc--> dtb---uboot-->kernel-->device_node--->platform_device结构体，里面包含各种资源

	
	2，设备树的具体作用？
	前面描述了，Linus本人受不了大量的硬件相关的代码，所以很明显，dts的出现，是为了解决这个问题。
	dts描述了计算机也就是Linux内核中特定的硬件设备信息的数据结构。

	3，dts数据结构的表现形式
	由一系列被命名的节点node和属性property组成，这些属性实际使用name+value的对子来描述。
	请注意，不同的平台，编写的总线设备驱动模型不一样，设备树的属性值可能也不一样，这取决于平台
	编写驱动的人，已经编写好的平台驱动的代码。
	我们有时候可以在内核自带的文档中找到相对应平台的设备树历程。

	4，使用在哪些方面？服务器也在使用dts吗？
	答：dts一般针对嵌入式系统，当然也会与一些服务器系统一起使用。
	然后，x86架构的计算机，一般不使用设备树，而是依靠各种自动配置协议来识别硬件。

二、dts/dtsi的编译工具dtc
	kernle/script/dtc/*


三、dtb

	dts/dtsi --------------dtc-------------------------> dtb
							      |
							      V
						通过bootloader加载到内核---dtb内存地址--> kernel


四、设备树的存储格式

	继承了Open Firmware IEEE 1275设备树的定义
	有三部分组成：头 header、结构块 struct block、字符串块 strings block。
	

		1，header
		描述设备树的基本信息，设备树大小、结构块偏移地址、字符串块偏移地址等。
		struct boot_param_header{...};(头部有个magic，是一个做校验的固定值，确定dtb是否正确)


	dtb	2，struct block
		描述node，比如node开始、结束等信息


		3，stings block
		描述属性，name + value，这里会使用一个属性表，用字符串块中的偏移量来映射属性值地址


五、如何加载dtb和启动内核

	资料1，u-boot命令行启动Linux内核
		load mmc 0 0x88000000 /boot/am335x-evm.dtb  
		load mmc 0 0x82000000 /boot/zImage  
		bootz 0x82000000 - 0x88000000  
		这样，内核就可以在解析dtb的情况下启动。
	自我分析:从上面的三条指令可以看到，首先我们现在内存中的某个地址，去加载dtb，然后再到内存中
	的另外一个地址加载内核，然后使用bootz来启动dtb和kernle。
	但是这里的细节无法得知？load具体做了什么，bootz具体做了什么？

	
	资料2，内核中的驱动如何找到对应的dts？
	dtb在启动阶段就加载进来，并分配到某一个内存地址。
	启动内核，驱动里面就会调用dts相关的api，去到dtb对应的内存地址寻找匹配的字符串信息。
	不解渴，继续寻找答案。

	资料3，uboot的代码去查找dtb如何load到内存
	① 在uboot中的boards.cfg里面，定义了一个DEFAULT_FDT_FILE的变量
	② DEFAULT_FDT_FILE = "***.dtb", DDR_MB=1024, SYS_USE_SPINOR,关联到dtb
	③ 然后在board.c中设置环境变量前，执行run loadfdt把dtb加载到内存中
	总的来说就是uboot会使用一个宏定义来关联dtb，然后在设置启动环境前，加载到内存中。

	资料4，https://blog.csdn.net/ooonebook/article/details/53206623
	这个帖子深入代码，而且可以追到uboot编译的各个阶段

	资料5，uboot传递dtb给内核
	1，高级版本的uboot也有了自己使用设备树支持，我们这里不讨论 uboot 使用的设备树
	2，Documentation/arm/Booting描述，解释dtb，必须提供两个信息，参数列表或者dtb镜像地址
		1）传递参数列表给内核
		2）bootm 内核地址 内存文件系统地址 dtb地址，启动内核

	总结就是uboot在启动内核的时候，传递dtb的地址给内核去启动。

	资料6，dtb地址的选择
	使用mkimage -l arm/arm64/boot/uImage，可以查看image文件的头部，也就是内核加载的地址
	（需要安装工具sudo apt install -y u-boot-tools）
	在选择dtb的地址有三个原则：
	1，不能破坏uboot本身
	2，内核本身的空间不能占用
	3，内核启动的页表空间也不能占用
	那么，dtb存在哪？
	答：存在内存的基址，比如0x3000 0000
	总结:我们可以通过mkimga的uboot tools的工具来查看内核镜像内存分布状态，获取头部信息，然后
	选择一个不会破坏uboot本身或者内核用到的内存的剩余内存空间，一般在基址处加载dtb。

	
六、内核对设备树的处理(获取dtb指针)
	前面描述了，uboot通过指令的方式从某个地址，传递dtb给内核，并启动内核。
	uboot设置R2寄存器，填写dtb的开始地址。
	那么我们就从内核第一个执行文件head.S来分析，内核对dtb的处理。
	1，head.S
		a. __lookup_processor_type : 使用汇编指令读取CPU ID,
		 根据该ID找到对应的proc_info_list结构体(里面含有这类CPU的初始化函数、信息)	
		b. __vet_atags : 判断是否存在可用的ATAGS或DTB（R2寄存器）
		c. __create_page_tables : 创建页表, 即创建虚拟地址和物理地址的映射关系
		d. __enable_mmu : 使能MMU, 以后就要使用虚拟地址了
		e. __mmap_switched : 上述函数里将会调用__mmap_switched
		f. 把bootloader传入的r2参数, 保存到变量__atags_pointer中
		g. 调用C函数start_kernel

		总结：bootloader首先设置一个宏来传递dtb，然后设置地址，保存到R2寄存器，去启动内核。
		内核启动的第一个执行文件head.S中，就会获取这个r2内指针的值，保存起来，再启动内核。

七、内核如何选择对应的machine_desc?
	每个内核镜像，对应一个单板设计，简单的说，就是每一个内核镜像，可能对应某个平台某个板子。
	那么在uboot传递dtb的时候，怎么确定内核的单板呢？注意，这个单板构造一个machin_desc结构体

	答：
	请看我们设备树的文件的根节点，有两行信息，比如：
	model = "****";
	compatible = "××"，"××"，"××";
	这个有点像高通平台中cdt的兼容性的设计，可以看出，cdt应该在解释不同的dts。
	这些compatible属性其实就是声明了machine_desc。


八、start_kernel的调用过程
	前面描述了，uboot-kernel的head.S-start_kernle
	那么这个start_kernel做了哪些事情？

	start_kernel--->setup_arch--->setup_machine_fdt(__atags_pointer);//设置我们dtb的指针
	--->early_init_dt_verify判断dtb是否有效--->of_flat_dt_match_machine找到machine_desc
	...

九、对设备树配置信息的处理

	总的来说，设备树只是对特定的硬件信息做一个封装，内核最终会解析dtb的设备信息，并赋值给
	驱动中的对应的变量。

	
可以继续阅读的文章收藏：
http://www.wowotech.net/linux_kenrel/why-dt.html






	
	
