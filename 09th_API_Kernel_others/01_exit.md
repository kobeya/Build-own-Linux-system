在函数结束的时候，我们常常使用_exit(),exit()来作为函数退出的符号，
那么这个exit()到底做了什么事情？

这里先提到一个：exit(0);
	表示当前进程正常退出，并且发送（如果有的话）17信号给父进程，
	效果等于：
		return 0; + kill(getppid(), 17);

	简单的说，就是_exit直接退出，不管缓冲区的文件是否写回。
