#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

#if 0	//it's question,why man fork,we can get throw inc, but it does'n neet ?
				|
				^
				because others lib inc it!!!
#include <sys/types.h>
#include <unistd.h>
#endif

int main()
{
	fork();//when run fork,then it become two processes,Parent and sun
	printf("hello\n");// two fork,so run twice
	fork();	//parent become two,sun also become two,now,we have four processes!
	printf("hello\n");//run four times,so the results of six print !
	while(1);
	return 0;
}
