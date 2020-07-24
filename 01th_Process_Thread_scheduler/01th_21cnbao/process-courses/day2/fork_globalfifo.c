#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/of_device.h>

struct globalfifo_dev {
	unsigned int current_len;
	unsigned char mem[GLOBALFIFO_SIZE];
	struct mutex mutex;
	wait_queue_head_t r_wait;
	wait_queue_head_t w_wait;
	struct fasync_struct * async_queue;
	struct miscdevice miscdev;
};

/* size_t 是为了方便系统移植而定义的，不同的体系，size_t可能不同。
 * 1，在32bit的系统上，定义为unsigned int，32 bit无符号整形
 * 2，在64bit的系统上，定义为unsigned long,64 bit无符号整形
 *
 * ssize_t 是signed size_t的扩展，表示有符号size_t
 * */

/* 1, __user，用来描述这个变量在用户空间，内核访问的时候要注意访问的安全问题。
 * 这里char __user *buf表示，内核要把文件读到用户的buf。
 * 2，loff_t *ppos,暂时不是很理解，查阅到资料，这个表示文件操作之后的读写位置偏移量？
 *	
 * */
static ssize_t globalfifo_read(struct file *filp, char __user *buf,
				size_t count, loff_t *ppos)
{
	int ret = 0;
	/* container_of,这个函数的原型：
	 * #define container_of(ptr, type, member) ({              \
		const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
		(type *)( (char *)__mptr - offsetof(type,member) );}) 

	 * container_of(ptr, type, member)
	 * 解析：表示已知结构体type的成员member的地址ptr，求解结构体type的起始地址
	 *
	 *
	 * */
	struct globalfifo_dev *dev = container_of(filp->private_data,
						struct globalfifo_dev, miscdev);

}




static const struct file_operations globalfifo_fops = {
        .owner = THIS_MODULE,
        .read = globalfifo_read,
        .write = globalfifo_write,
        .unlocked_ioctl = globalfifo_ioctl,
        .poll = globalfifo_poll,
        .fasync = globalfifo_fasync,
        .open = globalfifo_open,
        .release = globalfifo_release,
};

static int globalfifo_probe(struct platform_device *pdev)
{
	struct globalfifo_dev *gl;
	int ret = 0;
	/*这里内核使用devm_kzalloc来分配内存，这个内存分配函数有个特点：
	 *在设备拆除或者卸载的时候，这部分的内存会自动释放。
	 * */
	gl = devm_kzalloc(&pdev->dev, sizeof(*gl), GFP_KERNEL);

	if(!gl)
		return -ENOMEM;//这个Linux的错误码表示内存溢出。
	gl->miscdev.minor = MISC_DYNAMIC_MINOR;
	gl->miscdev.name  = "globalfifo";
	gl->miscdev.fops  = &globalfifo_fops;

	/*动态初始化一个互斥体,注意哦，互斥体跟互斥信号量是类似却实际不同的锁。
	*初始化，加锁解锁等接口都不一样。
	*/
	mutex_init(&mutex);//动态初始化一个互斥体

	init_waitqueue_head(&gl->r_wait);
	init_waitqueue_head(&gl->w_wait);
	platform_set_drvdata(pdev, gl);

	ret = misc_register(&gl->miscdev);
	if(ret < 0)
		goto err;
	dev_info(&pdev->dev, "globalfifo drv probed\n");
	return 0;
err:
	return ret;

}

static int globalfifo_remove(struct platform_device *pdev)
{
        struct globalfifo_dev *gl = platform_get_drvdata(pdev);

        misc_deregister(&gl->miscdev);

        dev_info(&pdev->dev, "globalfifo drv removed\n");
        return 0;
}

static struct platform_driver globalfifo_driver = {
	.driver = {
		.name  = "globalfifo",
		.owner = THIS_MODULE,
	},
	.probe 	= globalfifo_driver,
	.remove = globalfifo_remove,
};

module_platform_driver(global_driver);
MODULE_AUTHOR("kobey@3nod");
MODULE_LICENSE("GPL V2");

