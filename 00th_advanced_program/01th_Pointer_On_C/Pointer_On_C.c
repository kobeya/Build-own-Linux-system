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


	/*while break and continue*/
	int temp = 0;
	while(temp < 2)
	{
		printf("while temp = %d\n", temp++);
		continue;
		printf("while temp end!\n");
	}

	for(temp = 0; temp < 2; temp++)
	{
		printf("for temp = %d\n", temp);
		continue;
		printf("for temp end!\n");
	}

	/*do{}while(1);*/
	do{
		printf("temp = %d\n", temp++);
	}while(temp < 3);


	/*goto*/

	{
		int temp1 = 0;
		for(temp1 = 0; temp1 < 3;)
		{
			printf("temp1 = %d\n", temp1);
			goto GotoHere;
		}
		printf("end temp1 for!\n");
		printf("goto middle\n");
	GotoHere:
		printf("goto temp1 = %d\n", temp1++);
		printf("1\n");
	}

	printf("main function end!\n");
	return 0;
}
