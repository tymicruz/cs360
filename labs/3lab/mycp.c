#include <fcntl.h>
#define BLKSIZE 4096
int fd, gd;

char buf[4096];

int main(int argc, char* argv[])
{
	int n = 0, total = 0;
	if(argc < 3){exit(1);}

	fd = open(argv[1], O_RDONLY);//open file to copy for read mode
	if(fd < 0){exit(2);}

	gd = open(argv[2], O_WRONLY | O_CREAT);

	if(gd < 0){exit(3);}

	while(n = read(fd, buf, BLKSIZE))
	{
		write(gd, buf, n);
		total+=n;
	}
	
	printf("successfully copied %d bytes from %s to %s", total, argv[1], argv[2]);

return 0;

}
