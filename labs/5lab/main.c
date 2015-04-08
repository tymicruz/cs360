#include "h.h"


int main(int argc, char* argv[], char* env[])
{
	//char tester[32] = "hello/this/is/a/test";
	char line[MAX_PATH_LEN] = "";
	char *myargv[MAX_PATH_PIECES]={0};
	int myargc, i;

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
		if(strcmp(myargv[0], "pwd") == 0)
		{
			printf("%s\n",pwd());
		}
		if(strcmp(myargv[0], "quit") == 0)
		{

		}

	}

}
