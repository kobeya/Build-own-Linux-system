#include "stdio.h"

#define	SIZE	50

int x[SIZE], y[SIZE];
int i, *p1, *p2;

static void try1(void);

char const *string = "hello kobey";

char buffer[] = {0};


char buffern[2][3] = {1, 2, 3, 4, 5, 6};

static void defstrcpy(char *buffer, char const *string);

void main()
{
	/*声明10个元素的整形常亮指针array，在后续访问中以下标0开始运算*/
	int array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	printf("array[9] = %d, array + 9 = %d\n", array[9], *(array + 9));

	/*一个奇怪但有合法的用法：转化为间接访问 *(2 + array)*/
//	array++;

	printf("array[2] = %d\n", 2[array]);
	defstrcpy(buffer, string);
	printf("buffer = \"%s\"\n", buffer);
	printf("buffern[1][2] = %d\n", *(*(buffern + 1) + 2));
}

static void try1()
{
	for(int j = 0; j < SIZE; j++)
	{
		x[j] = y[j];
	}
}


static void defstrcpy(char *buffer, char const *string)
{
	while((*buffer++ = *string++) != '\0')
		;
}
