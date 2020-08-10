#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int fd = -1;
	int ret = -1;
	int process_inter = 0;
	fd = open("./myfifo", O_WRONLY);
	if(fd < 0) {
		printf("A process open myfifo failure\n");
	}
	printf("A process open myfifo sucess\n");
	process_inter = 1;
	ret = write(fd, &process_inter, 1);
	if(ret < 0) {
		printf("A process write failure\n");
	}
	printf("A process write sucess\n");

	return 0;
}
