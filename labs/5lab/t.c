#include <h.c>

char *disk = "adisk";

main(int argc, char* argv[])
{
	if(argc > 1)
		disk = argv[1];

	fd = open(disk, O_RDWR);

	if(fd < 0)
	{
		printf("couldn't open %s\n", disk);
		exit(1);
	}
}
