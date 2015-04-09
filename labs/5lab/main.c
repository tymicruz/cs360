#include "h.h"


int main(int argc, char* argv[], char* env[])
{
	//char tester[32] = "hello/this/is/a/test";
	char line[MAX_PATH_LEN] = "";
	char *myargv[MAX_PATH_PIECES]={0};
	int myargc, i;
	/*char test1[100] = "/";
	char test2[100] = "/";

	printf("%s\n", dirname(test1));
	printf("%s\n", dirname(test2));
getchar();*/

	if(argc > 1)
	{
		disk = argv[1];
	}

	init();
	mount_root();
/*	//printf("%s\n", tester);
	printf("%s\n", basename(tester));
	//printf("%s\n", basename(tester));
	tester[strlen(tester) - strlen(basename(tester)) - 1] = 0;
	printf("%s\n", basename(tester));
	//printf("%s\n", baseNameChopper(tester));*/
	printf("pwd:%s\n",pwd());

	while(1)
	{
		printf("\ntyos (/%s):", (char*)basename(pwd()));


		bzero(line, MAX_PATH_LEN);
		__fpurge(stdin);
		fgets(line, MAX_PATH_LEN, stdin);

		line[strlen(line) - 1] = 0;
		tokCmd(line, myargv, &myargc);

		if(!myargc) continue;

		printf("%s\n", myargv[0]);

		if(strcmp(myargv[0], "ls") == 0)
		{
			if(myargc < 2){
				ls_wrap(0);//ls this is a wrapper for ls
			}
			else{
	
				ls_wrap(myargv[1]);//ls this is a wrapper for ls
			}
		}
		if(strcmp(myargv[0], "cd") == 0)
		{
			if(myargc < 2){
				cd_wrap(0);
			}
			else{
	
				cd_wrap(myargv[1]);
			}
		}
		if(strcmp(myargv[0], "mkdir") == 0)
		{
			if(myargc < 2){
				mkdir_wrap(0);
			}
			else{
	
				mkdir_wrap(myargv[1]);
			}
		}
		if(strcmp(myargv[0], "rmdir") == 0)
		{
			if(myargc < 2){
				rmdir_wrap(0);
			}
			else{
	
				rmdir_wrap(myargv[1]);
			}
		}
		if(strcmp(myargv[0], "unlink") == 0)
		{
			if(myargc < 2){
				unlink_wrap(0);
			}
			else{
	
				unlink_wrap(myargv[1]);
			}
		}
		if(strcmp(myargv[0], "creat") == 0)
		{
			printf("creating\n");
			if(myargc < 2){
				creat_wrap(0);
			}
			else{
	
				creat_wrap(myargv[1]);
			}
		}
		if(strcmp(myargv[0], "pwd") == 0)
		{
			printf("%s\n",pwd());
		}
		if(strcmp(myargv[0], "quit") == 0)
		{
			quit();
		}
		if(strcmp(myargv[0], "imap") == 0)
		{
			i = ialloc(fd);

			if(i < 0)
			{
				printf("no more inodes to alloc");
			}
			else
			{	printf("inode %d is next available.\n", i);
				imap(fd);
				idealloc(fd, i);
			}
			imap(fd);
		}
		if(strcmp(myargv[0], "bmap") == 0)
		{
			i = balloc(fd);

			if(i < 0)
			{
				printf("no more blocks to alloc");
			}
			else
			{	printf("block %d is next available.\n", i);
				bmap(fd);
				bdealloc(fd, i);
			}
			bmap(fd);
		}
		if(1){
		}

	}

}
