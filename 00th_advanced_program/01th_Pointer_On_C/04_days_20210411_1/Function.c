#include "stdio.h"

static int *func1(int argc, char *argv[]);
static void bi_to_ascii(unsigned int;);
void main(void)
{
	func1(1, "hello");

	bi_to_ascii(4267);
}

static int *func1(int argc, char *argv[])
{
	printf("argc = %d, argv[] = %s\n", argc, argv);
	return 0;
}

static void bi_to_ascii(unsigned int value)
{
	unsigned int tmp;
	tmp = value / 10;
	if(tmp != 0)
	{
		bi_to_ascii(tmp);
	}
	putchar(value % 10 + '0');
	printf("1\n");
}
