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

	/*调用DECLARE_WAITQUEUE()创建一个等待队列的项*/
	DECLARE_WAITQUEQUE(wait, current);
	
	/*获取一个互斥体，也就是信号量中的互斥锁，可以睡眠*/
	mutex_lock(&dev->mutex);
	
	/*将一个等待队列的项，添加到等待队列，该函数在获得必要的锁(互斥体)后，
	 * 使用 __add_wait_queue() 函数来完成队列添加工作。
	 * 这里获得锁之后，把dev中的r_wait加到刚创建的等待队列中
	 * */	
	add_wait_queue(&dev->r_wait, &wait);
	/*这里做一个判断，如果当前的队列中没有任务在等待，那么就进到循环体中*/	
	while (dev->current_len == 0) {
		/*O_NONBLOCK非阻塞*/
		if (filp->f_flags & O_NONBLOCK) {
		/*读不到任何等待进程，那么就EAGAIN出去，表示现在没有东西，待会再来*/
			ret = -EAGAIN;
			goto out;
		}
		/*进程当前还不能运行，那么就挂起在可中断的睡眠状态*/
		__set_current_state(TASK_INTERRUPTIBLE);
		/*睡眠，就是主动放弃CPU资源，那么就要释放锁*/
		mutex_unlock(&dev->mutex);
		/*这个函数做了非常多的事情，需要用一个文件，单独描述这个函数调用，详细请看路径
		 * 这里简单的说，就是把阻塞的进程，设置为可中断的睡眠状态，然后主动放弃CPU，
		 * 让CPU去调度其他进程，直到它返回，再去判断返回的是信号，还是资源。
		 * */
		schedule();
		/*检查当前进程是否有信号处理，返回不为0表示有信号需要处理。
		 * 返回 -ERESTARTSYS 表示信号函数处理完毕后重新执行信号函数前的某个系统调用。
		 * 也就是说，如果信号函数前有发生系统调用，在调度信号处理函数之前,
		 * 内核会检查系统调用的返回值，看看是不是因为这个信号而中断了系统调用.
		 * 如果返回值-ERESTARTSYS,并且当前调度的信号具备-ERESTARTSYS属性,
		 * 系统就会在用户信号函数返回之后再执行该系统调用。
		 * */		
		if (signal_pending(current)) {
			ret = -ERESTARTSYS;
			goto out2;
		}

		mutex_lock(&dev->mutex);
	}

	





out:
	mutex_unlock(&dev->mutex);
out2:
	remove_wait_queue(&dev->w_wait, &wait);
	set_current_state(TASK_RUNNING);
	return ret;







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
	mutex_init(&mutex);//动态初始化一个互斥体,初始化后，就可以使用mutex_lock、mutex_unlock。

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

