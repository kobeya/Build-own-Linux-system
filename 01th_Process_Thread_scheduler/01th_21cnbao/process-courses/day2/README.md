# 编译运行cow.c的例子，理解data变化的原理，解释打印的为什么是10,20,10
# 编译运行vfork.c的例子，理解data变化的原理，解释打印的为什么是10,20,20
# 编译gcc thread.c -pthread,用strace ./a.out跟踪其对clone()的调用
# 编译运行life_period.c，杀死父进程，用pstree命令观察子进程的被托孤
# 仔细阅读globalfifo.c的global_read()、globalfifo_write()函数，理解等待队列


#strace:

1,strace常用来跟踪进程执行时的系统调用和所接收的信号
2,strace可以跟踪到一个进程产生的系统调用,包括参数，返回值，执行消耗的时间
3,strace后打印的信息解读：左边系统调用的函数名及其参数，
	等号右边是调用后的返回值。
