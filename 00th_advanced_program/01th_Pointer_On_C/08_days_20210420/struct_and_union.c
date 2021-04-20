#include <stdio.h>

int main()
{
	union {
		int a;
		int b;
	} temp = { 50.1 };

	printf("b = %d\n", temp.b);
	return 0;
}
