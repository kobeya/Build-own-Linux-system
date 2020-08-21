#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


int main()
{
	printf("pid = %d\n", getpid());
	while(1);
	return 0;	
}
