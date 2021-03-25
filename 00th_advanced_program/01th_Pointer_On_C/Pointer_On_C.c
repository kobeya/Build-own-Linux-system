#include <stdio.h>
int
main(int argc, char *argv[], char *envp[])
{
	unsigned int temp1 = 1;
	switch(temp1)
	{
		case 1:
			printf("Hello, kobey!\n");
			break;
		defualt:
			;
			break;
	}
#if 0
	/*print env per line*/
	unsigned int i = 0;
	while(envp[i] != NULL)
	{
		printf("%s\n", envp[i]);
		i++;
	}
#endif
	enum E_type {
		CPU0,
		CPU1 = 10,
		CPU2
	};

	printf("CPU1 = %d\n", CPU1);

	/*define pointer*/

	char *message = "hello,world";	

	typedef unsigned short us;
	us kobe = 10;
	printf("kobe = %d\r\n", kobe);

	return 0;
}
