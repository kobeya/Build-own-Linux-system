#include "stdio.h"
void main()
{
	#if 0//can not access Null P
	int *p = NULL;
	*p = 20;
	printf("Pointer P -> %d\n", *p);
	#endif

	#if 0
	*(int *)1000 = 12;
	#endif


	char *temp = "hello";
	for(char *p = temp; p < temp + 5;)
	{
		printf("%c\t", *p);
		p++;
	}

	for(char *p = temp + 4; p >= temp;)
	{
		printf("%c\n", *p);
		--p;//in fact,here stash a BUG:can not access address before the first element of the array
	}
}
