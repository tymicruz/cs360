/*************** a.c file **************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ARGS 32
int removeNewLine(char *s);
int pid;

char line[128];
char token[32][64];
char cwd[128], command[128];

main(int argc, char *argv[ ])
{
	int i = 0;
	int myargc = 0;
	char *word;
  //1. DRAW A DIAGRAM TO SHOW EXACTLY WHAT IS argv:
	
	// DEFINE myargv as AN ARRAY OF 32 char pointers:
	char* myargv[32];

 // 2. Write code to           |-------- n=5--------| 
   //     input a line, e.g.    b.out this is a test
     //   tokenize line into inidividual strings as 
       //    token[0], token[1],....,token[n-1]
	printf("cmd? ");
	fgets(line, 128, stdin);
	__fpurge(stdin);
	removeNewLine(line);
	word = strtok(line, " ");
	i = 0;	
	while(word)
	{
		myargc++;
	  //	printf("size of: %d\n", strlen(word));
	  	strcpy(token[i++], word);
		word = strtok(NULL, " ");
	}
 // 3. // Write code to let myargv[i] point to token[i], (0 <= i < n)
   for(i = 0; i < myargc; i++)
	{
		myargv[i] =  token[i];
	}

	myargv[i] = 0;
  	pid = getpid();
  	printf("proc %d in a.out exec to b.out\n", pid);
  	getcwd(cwd,128);     // cwd contains the pathname of CWD  
  	//printf("cwd = %s\n", cwd);
  // WRITE CODE to let command = CWD/b.out
  	strcat(cwd, "/b.out");
	strcpy(command, cwd);
	printf("command = %s\n", command);
 	execv(command, &myargv);
	//execl("b.out", "hello", "world");
	printf("execve failed\n");
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
 

