#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main (void)
{
	char buf[1024];
	int fd1, fd2;
	fd1 = open("tiny", O_RDONLY);
	fd2 = open("tiny", O_RDONLY);
	
	bzero(buf, 1024);
	lseek(fd1, 10, 0);
	read(fd1, buf, 20);
	printf("%s\n", buf);
	bzero(buf, 1024);
	lseek(fd1, 10, 0);
	read(fd1, buf, 5);
	printf("%s\n", buf);
	/*read(fd2, buf, 20);
	printf("%s\n", buf);bzero(buf,1024);
	lseek(fd2, buf, 20);
	printf("%s\n", buf);*/
}
