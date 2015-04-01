/*************** a.c file **************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define ARGS 32
int removeNewLine(char *s);
int hasPipe(char* line);
int pid;

char line[128];
char path_line[128];
char home_line[128];
char token[32][64];
char cwd[128], command[128], hd[128];
char action[32];
int pd[2];
char *writer = 0, *reader = 0;
char *myargv[32], *writerargv[32], *readerargv[32];

main(int argc, char *argv[ ], char *env[])
{
	int i = 0, j = 0, status = 0, r;
	int num_paths = 0;
	int myargc = 0;
	char *word;
	char *path;//for finding path in env
	char *rest;//rest of string after PATH=
	char* myenv[64];
	char  paths[32][128];
	char *pipe_cmds[32];
	int pid2;

//get current working directory
	getcwd(cwd, 128);

	//find path in env[]
	while(env[i])
	{
		strcpy(line, env[i]);
		path = strtok(line, "=");
		//printf("%s\n", *env);
		
		if(strcmp(path, "PATH") == 0)
		{
			strcpy(path_line, env[i]);
			//env++;
		}
		else if(strcmp(path, "HOME") == 0)
		{
			strcpy(home_line, env[i]);
			//env++;
		}
		else
		{
			while(strtok(NULL, "="));
			//env++;
			//i++;
		}
		i++;
	}

	//env = env - i;//move env pointer back to original location

	i = 0;
	path = strtok(path_line, ":");
	//get all paths
	while(path)
	{
		strcpy(paths[i], path);
	//	printf("paths[%d]:%s\n", i, paths[i]);
	   path = strtok(NULL, ":");
		i++;
	}

	//remove path from the first path
	strcpy(paths[0],  strtok(path_line, "="));
	strcpy(paths[0], strtok(NULL, "="));

	//add current working directory
	strcpy(paths[i], cwd);
	i++;
	num_paths = i;

	printf("show PATH=\n");
	for(i = 0; i < num_paths; i++)
	{
		printf("%-2d)%s\n", i, paths[i]);
	}

//get home directory
	strcpy(hd, strtok(home_line, "="));
	strcpy(hd, strtok(NULL, "="));

	printf("show HOME directory=(%s)\n", hd);
	printf("****cruzsh processing loop initiated****\n");

while(1){	
	i=0;j=0;

	//get line
	printf("tcsh % ? ");
	fgets(line, 128, stdin);
	__fpurge(stdin);//clear stdin
	removeNewLine(line);

	if(hasPipe(line))
	{
		if(getHeadandTail(line) < 0)
		{
			continue;
		}
		goto PIPES;
	}

	word = strtok(line, " ");
	myargc = 0;

	//tokenize users input
	while(word)
	{
		myargc++;
	  	strcpy(token[i++], word);
		word = strtok(NULL, " ");
	}

	//new argv's
   for(i = 0; i < myargc; i++)
	{
		myargv[i] =  token[i];
	}
	myargv[i] = 0;//set last pointer to null

	if(myargv[0] == 0)
	{
		//if dude didn't type anything (just hit enter, skip to next loop iteration
		continue;
	}

	//child will do this
	if(strcmp(myargv[0], "exit") == 0)
	{
		exit(1);
	}
	if(strcmp(myargv[0], "cd") == 0)
	{
		if(myargc > 1)
		{
			chdir(myargv[1]);
			printf("cd to %s\n", myargv[1]);
		}
		else
		{
			chdir(hd);
			printf("cd to %s\n", hd);
		}

		continue;
	}
	else
	{	
		pid = fork();
	}

	if(pid)//parent waits for child to finish
	{
		//printf("My parent is %d\n", getppid());
		//printf("I am %d  waiting for my kid, (%d)\n", getpid(), pid);
		wait(&status);
		//printf("%d kid is done\n", pid);
	}
	else//child does this
	{
		for(i = 0; i < num_paths; i++)
		{
			strcpy(command, paths[i]);
			//add '/' to end if not there
			if(command[strlen(command) - 1] != '/')
			{
				strcat(command, "/");
			}

			strcat(command, myargv[0]);
			printf("%s?", command);
			//getchar();
			execve(command, myargv, env);//or execve(command,&myargc, &myenv);
			printf(" failed\n");
		}
		printf("uknown command (%s)\n", myargv[0]);
		//printf("line:%s\n", line);
		exit(1);
	}

	continue;
 PIPES:
 	i = 0;
	while(writerargv[i])
	{
		printf("w%d %s\n", i, writerargv[i]);
		i++;
	}

	i = 0;
	while(readerargv[i])
	{
		printf("r%d %s\n", i, readerargv[i]);
		i++;
	}

//	r = pipe(pd);

	pid = fork();

	if(pid)//parent shell will wait
	{
		printf("waiting for %d\n", pid);
		wait(&status);
	}
	else//child of my shell will fork another process to be a reader
	{
		printf("im the child pipe proc\n");

		r = pipe(pd);

		pid = fork();//make reader and writer

		if(pid)//writer
		{
			printf("I am the writer, %d, child of %d\n", getpid(), getppid());
	
			close(pd[0]);
			close(1);
			dup(pd[1]);

			for(i = 0; i < num_paths; i++)
			{
				strcpy(command, paths[i]);

				if(command[strlen(command) - 1] != '/')
				{
					strcat(command, "/");
				}
				
				strcat(command, writerargv[0]);
				printf("%s\n", command);
				execve(command, writerargv, env);
			}
			wait(&status);
			exit(4);
			//printf("wtf is %s?\n", writerargv[0]);
		}
		else//reader
		{
		//getchar();
		close(pd[1]);
		close(0);
		dup(pd[0]);
		printf("I am the reader, %d, child of %d\n", getpid(), getppid());

			for(i = 0; i < num_paths; i++)
			{
				strcpy(command, paths[i]);

				if(command[strlen(command) - 1] != '/')
				{
					strcat(command, "/");
				}

				strcat(command, readerargv[0]);
				printf("%s\n", command);
				execve(command, &readerargv, env);
			}
					
			exit(4);
		}
		printf("impossible?\n");
	}

	}//while loop
}

 int removeNewLine(char *s)
 {//this function only removes a newline if it comes directly before the null char
     int loops = 0;
  
     while(*s)//go to the end of the string (only remove ending new line)
     {
        s++;
        loops++;
     }
  
     if(loops)
     {
        s--;//go back one char
        if(*s == '\n')
        {
           *s = 0;
        }
     }
  }
int hasPipe(char *line)
{	
	while(*line)
	{
		if(*line == '|')
		{
			return 1;
		}
		line++;
	}

	return 0;
}

int getHeadandTail(char *line)
{
	char *parts[2];
	char *part = 0;
	int i = 0;

	part = strtok(line, "|");

	while(part && i < 2)
	{
		parts[i] = part;
		part = strtok(NULL, "|");
		i++;
	}
	
	//there are not enough pieces for head and tail
	if(i < 1)
	{
		return -1;
	}

	writer = parts[0];
	reader = parts[1];
	
	i = 0;
	part = strtok(writer, " ");
	while(part)
	{
		writerargv[i] = part;
		part = strtok(NULL, " ");
		i++;
	}
	writerargv[i] = 0;

	i = 0;
	part = strtok(reader, " ");
	while(part)
	{
		readerargv[i] = part;
		part = strtok(NULL, " ");
		i++;
	}

	return 0;
}
