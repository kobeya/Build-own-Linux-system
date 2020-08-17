#if 0
一、指针函数
	int *function();
	int* function();
	这个function函数的返回值是一个整形指针，也就是这个函数，会返回一个地址。

二、函数指针
	int (*funtion)();






#endif

#include <stdio.h>
#include <stdlib.h>

void func(int num)
{
	printf("func%d\n", num);
}

int main()
{
	void (*p)(int);
	printf("func = %ld, &fun = %ld, *fun = %ld\n", func, &func, *func);
	p = func;//函数名本身可以表示函数存储的首地址，也可以表示函数的指针体，里面也存储着函数的地址
	p(1);
	p = &func;
	p(2);
	return 0;
}	
