#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int fd = -1;
	int ret = -1;
	int process_inter = 0;
	fd = open("./myfifo", O_RDONLY);
	if(fd < 0) {
		printf("B process open myfifo failure\n");
	}
	printf("B process open myfifo sucess\n");
	ret = read(fd, &process_inter, 1);
	if(ret < 0) {
		printf("B process read failure\n");
	}
	printf("B process read sucess\n");
	sleep(5);
	while(process_inter == 0);
	printf("B process end, process_inter = %d\n", process_inter);

	return 0;
}
