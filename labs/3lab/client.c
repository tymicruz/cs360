//**************************** ECHO CLIENT CODE **************************
// The echo client client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>//errno strerror(errno)
#include <sys/socket.h>

#include <sys/stat.h>//idk stat?
#include <sys/types.h>//idk stat?
#include <unistd.h>//idk stat:w

#include <time.h>
#include <dirent.h>

#include <netdb.h>
#include <fcntl.h>//used for 0_RDONLY

#define MAX 256

// Define variables
struct hostent *hp;              
struct sockaddr_in  server_addr;

int sock, r;
int SERVER_IP, SERVER_PORT; 

char cwd[MAX];//current working directory
char hd[MAX];//homedirectory

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int isLocal (char *cmd);
int isValid(char *cmd);
char* removeL(char *cmd);
int doLocalCmd(int argc, char *argv[]);
char* getHomeDir(char *env[]);
int listenToServer(char* cmd, int argc, char *argv[]);

int ls_file(char *fname);
int ls_dir(char *dname);

int fd;
FILE *fp;
// clinet initialization code

int client_init(char *argv[])
{
  printf("======= client init ==========\n");

  printf("1 : get server info\n");
  hp = gethostbyname(argv[1]);
  if (hp==0){
     printf("unknown host %s\n", argv[1]);
     exit(1);
  }

  SERVER_IP   = *(long *)hp->h_addr;
  SERVER_PORT = atoi(argv[2]);

  printf("2 : create a TCP socket\n");
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock<0){
     printf("socket call failed\n");
     exit(2);
  }

  printf("3 : fill server_addr with server's IP and PORT#\n");
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = SERVER_IP;
  server_addr.sin_port = htons(SERVER_PORT);

  // Connect to server
  printf("4 : connecting to server ....\n");
  r = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (r < 0){
     printf("connect failed\n");
     exit(1);
  }

  printf("5 : connected OK to \007\n"); 
  printf("---------------------------------------------------------\n");
  printf("hostname=%s  IP=%s  PORT=%d\n", 
         (char*) hp->h_name, (char*)inet_ntoa(SERVER_IP), SERVER_PORT);
  printf("---------------------------------------------------------\n");

  printf("========= init done ==========\n");
}

main(int argc, char *argv[ ], char *env[])
{
   int n, i, m;
   char line[MAX], linecopy[MAX], ans[MAX];
	char token[32][64];
	char *word;
	char *cmd;
	char *myargv[32];
	char myargc = 0;
	char s_or_fail[2];

 	 strcpy(hd, getHomeDir(env));

  if (argc < 3){
     printf("Usage : client ServerName SeverPort\n");
     exit(1);
  }

  client_init(argv);

  printf("********  processing loop  *********\n");
  while (1){
    printf("\ninput a line : ");
    bzero(line, MAX);                // zero out line[ ]
	 __fpurge(stdin);
    fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

    line[strlen(line)-1] = 0;        // kill \n at end
    if (line[0]==0){                  // exit if NULL line
     continue;}//exit(0);

		strcpy(linecopy, line);
//-----------tokenize linecopy (aka destroy linecopy and get pieces)
		word = strtok(linecopy, " ");
		i = 0;
		myargc = 0;
		while(word)
		{
			myargc++;
			strcpy(token[i++], word);
			word = strtok(NULL, " ");
		}

//printf("---->:%d\n", myargc);
//getchar();
		//myargc = i;

		for(i = 0; i < myargc; i++)
		{
			myargv[i] = token[i];
			//write(sock, myargv[i], MAX);
		}

		myargv[i] = 0;
		cmd = myargv[0];
		if(strcmp(cmd, "quit") == 0)
		{
			exit(0);
		}
//-----------done tokenizing line
    // Send ENTIRE line to server

	 if(!isLocal(cmd) && isValid(cmd))//have server to stuff
	 {
    	n = write(sock, line, MAX);
    	printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
		m = read(sock, s_or_fail, 2);

		if(strcmp(s_or_fail, "1") == 0)//sucess
		{
			printf("client listening to server:\n");
			listenToServer(cmd, myargc, myargv);

		}else if(strcmp(s_or_fail, "0") == 0)//fail
		{
			printf("server says, \"leave me alone\".\n");
		
		}else
		{
			printf("wtf\n");
		}
		
		//n = read(sock, ans, MAX);
    	//printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
	 }
	 else if(isLocal(cmd) && isValid(removeL(cmd)))//do stuff locally
	 {
		cmd = removeL(cmd);

		printf("doing %s locally\n", cmd);
		doLocalCmd(myargc, myargv);
	 }
	 else{
	 	printf("input cmd is not valid!\n");
		continue;
	 }
    
  }//end of while
}

int isLocal(char *cmd)
{
	int i = 0;

	if(strcmp(cmd, "ls") == 0)//if it's ls, it's not local
	{
		return 0;
	}

	//l is the first
	if(cmd[0] == 'l')
	{
		return 1;	
	}

	//l is not first
	return 0;
}

int isValid (char* cmd)
{
	if(cmd == 0)
	{
		return 0;
	}

	if(strcmp(cmd, "pwd") == 0)
	{
		return 1;
	}	if(strcmp(cmd, "ls") == 0)
	{
		return 1;
	}	if(strcmp(cmd, "cd") == 0)
	{
		return 1;
	}	if(strcmp(cmd, "mkdir") == 0)
	{
		return 1;
	}	if(strcmp(cmd, "rmdir") == 0)
	{
		return 1;
	}	if(strcmp(cmd, "rm") == 0)
	{
		return 1;
	}	if(strcmp(cmd, "get") == 0)
	{
		return 1;
	}	if(strcmp(cmd, "put") == 0)
	{
		return 1;
	}	
		
	if(strcmp(cmd, "cat") == 0)
	{
		return 1;
	}
	if(strcmp(cmd, "quit") == 0)
	{
		return 1;
	}

	return 0;
}

char* removeL(char *cmd)//remove the first char (used to remove 'l')
{
	int i = 1;
	char* noL = 0;//(char*)malloc(sizeof(char));

	if(strlen(cmd) <=  1)//if one or 0 chars (or less) return 0
	{
		return noL;
	}

	while(cmd[i])
	{
		noL = (char *) realloc (noL, (i+1) * (sizeof(char)));
		noL[i - 1] = cmd[i];
		noL[i++] = 0;
	}

	return noL;
}

int doLocalCmd (int myargc, char* myargv[])//command will not b null when called
{

	//function will only be called for local commands
	char *cmd = myargv[0];
	struct stat fstat, *sp;
	char *op = 0;
	int i, n;
	char buf[1024];

	cmd = removeL(cmd);
	sp = &fstat;

	if(myargc > 1)
	{
		
		op = myargv[1];
	}//otherwise op is 0
	
	if(strcmp(cmd, "pwd") == 0)
	{
		getcwd(cwd, MAX);
		printf("lpwd: %s\n", cwd);
		return 1;
	}	
	if(strcmp(cmd, "ls") == 0)
	{
		if((myargc > 1) && lstat(myargv[1], &fstat) < 0)
		{	
			printf("I couldn't lstat %s\n", myargv[1]);
			return 0;
		}else if(myargc > 1)
		{
			if(S_ISDIR(sp->st_mode))
			{
				ls_file(myargv[1]);
				return 0;
			}else{
				ls_file(myargv[1]);
				return 0;
			}
		}

		if(myargc == 1)
		{
			printf("ls current dir\n");
			ls_dir("./");
			return 0;
		}

		printf("wtf happen with lls\n");
		//need to ls all files
		
		return 1;
	}	
	if(strcmp(cmd, "cd") == 0)
	{
		if(!(chdir(op) < 0))//if argv available and chdir success
		{
			printf("lcd OK\n");			

		}else if(myargc == 1)
		{
			printf("lcd to HOME dir\n");
			chdir(hd);			
		}
		else
		{
			printf("lcd FAIL\n");
		}
		return 1;
	}	
	if(strcmp(cmd, "mkdir") == 0)
	{
		if(mkdir(op, 0777) < 0)
		{
			printf("lmkdir FAIL -> errno=%d : %s\n", errno, strerror(errno));
		}else
		{
			printf("lmkdir %s OK\n", op);
		}
		return 1;
	}	
	if(strcmp(cmd, "rmdir") == 0)
	{
		if(rmdir(op) < 0)
		{
			printf("lrmkdir FAIL -> errno=%d : %s\n", errno, strerror(errno));
		}else
		{
			printf("lrmkdir %s OK\n", op);
		}
		return 1;
	}	
	if(strcmp(cmd, "rm") == 0)
	{
		if(unlink(op) < 0)
		{
			printf("lrm FAIL -> errno=%d : %s\n", errno, strerror(errno));
		}
		else
		{
			printf("lrm %s OK\n", op);
		}
		return 1;
	}	
	if(strcmp(cmd, "get") == 0)
	{
		printf("use (get) instead.\n");		
		return 2;//need to work with server
	}	
	if(strcmp(cmd, "put") == 0)
	{
		printf("use (put) instead.\n");		
		return 2;
	}
	if(strcmp(cmd, "cat")==0)
	{
		if(myargc < 2)
		{
			printf("what do you want me to cat, dog?\n");
			return 1;
		}

		if(lstat(myargv[1], &fstat) < 0)
		{
			printf("I couldn't lstat %s\n", myargv[1]);
			return 0;
		}else
		{
			if(!S_ISREG(sp->st_mode))
			{
				printf("I only stat REG files, bro.\n");
				return 0;
			}
		}
		
		fd = open(myargv[1], O_RDONLY);

		if(fd < 0)
		{
			printf("can't cat this, man.\n");
			return 1;
		}

		while(n = read(fd, buf, 1024)){
			for(i = 0; i < n; i++)
			{
				putchar(buf[i]);
			}
		}
	
		close(fd);
	}
	if(strcmp(cmd, "quit") == 0)
	{
		exit(1);
	}

	return 0;
}

char *getHomeDir(char *env[])
{
	int i = 0;
	char line[MAX];
	char *hd = 0;

	//assuming HOME path exists
	while(env[i])
	{
		strcpy(line, env[i]);
		hd = strtok(line, "=");
		
		if(strcmp(hd, "HOME") == 0)
		{
			//assuming HOME=something
			hd = strtok(NULL, "=");
			break;
		}
		i++;
	}

	return hd;
}

int listenToServer(char* cmd, int myargc, char *myargv[])
{
	int n = 0, i = 0;
	char buf[1024];
	char response[MAX];
	char done[2];
	bzero(buf, 1024);

	if(myargc > 1 && strcmp(cmd, "get") == 0)//get with arg
	{
	//	fd = open(argv[1], O_WRONLY | O_CREAT);//open file
		fp = fopen(myargv[1], "w");//open file

		while((n = read(sock, buf, 1024)))
		{
			printf("bytes: %d\n", n);
			for(i=0; i < n; i++)
			{
				if(buf[i] == 0 || buf[i] == EOF)
				{
					bzero(buf, 1024);
					i = n;
					continue;
					break;
				}
				else
				{	
					putchar(buf[i]);
					fputc(buf[i], fp);
				}
			}

			read(sock, done, 2);//see it done

			if(strcmp(done, "0") == 0)
			{
				break;
			}
		}
		printf("done - server served you!\n");
		fclose(fp);
	}
	else if(strcmp(cmd, "put") == 0)
	{
		if(myargc <= 1) {write(sock, "0", 2); return 0;}//put fail

		fd = open(myargv[1], O_RDONLY);

		if(fd < 0)
		{
			write(sock, "0", 2);//put fail on file open
			return 0;
		}

		write(sock, "1", 2);//good put
		printf("putting local file: %s to server\n", myargv[1]);


		while(n = read(fd, buf, 1024))
		{
			write(sock, buf, 1024);
			if(n >= 1024)
			{
				write(sock, "1", 2);
			}
			else
			{
				write(sock, "0", 2);
			}
		}

		close(fd);
		return 2;//need to write more to server
	}
	else if(strcmp(cmd, "pwd") == 0)
	{
		n = read(sock, cwd, MAX);
		printf("server gives pwd: %s\n", cwd);
	}else if(strcmp(cmd, "cd") == 0)
	{
		n = read(sock, response, MAX);
		printf("cd server response: %s\n", response);
	}else if(strcmp(cmd, "mkdir") == 0)
	{
		n = read(sock, response, MAX);
		printf("mkdir server response: %s\n", response);
	}else if(strcmp(cmd, "rmdir") == 0)
	{
		n = read(sock, response, MAX);
		printf("rmdir server response: %s\n", response);
	}else if(strcmp(cmd, "rm") == 0)
	{
		n = read(sock, response, MAX);
		printf("rm server response: %s\n", response);
	}else if(strcmp(cmd, "cat") == 0){

		n = read(sock, response, MAX);
		printf("server response: %s\n", response);	
	}else if(strcmp(cmd, "ls") == 0){


		while(1)
		{
		
			n = read(sock, done, 2);

			if(strcmp(done, "0") == 0)
			{
				printf("server ls is done\n");
				break;
			}
			else if(strcmp(done, "1") == 0)
			{
				//printf("one file\n");
				n = read(sock, response, MAX);
				printf("server response: %s\n", response);
			}
			else
			{
				printf("wtf is happening on server\n");
				break;
			}
			
			
		}	
	}
	
	

}

int ls_file(char *fname)
{
  struct stat fstat, *sp;
  int r, i;
  char ftime[64];

  sp = &fstat;
  //printf("name=%s\n", fname); getchar();

  if ( (r = lstat(fname, &fstat)) < 0){
     printf("can't stat %s\n", fname); 
     exit(1);
  }

  if ((sp->st_mode & 0xF000) == 0x8000)
     printf("%c",'-');
  if ((sp->st_mode & 0xF000) == 0x4000)
     printf("%c",'d');
  if ((sp->st_mode & 0xF000) == 0xA000)
     printf("%c",'l');

  for (i=8; i >= 0; i--){
    if (sp->st_mode & (1 << i))
	printf("%c", t1[i]);
    else
	printf("%c", t2[i]);
  }

  printf("%4d ",sp->st_nlink);
  printf("%4d ",sp->st_gid);
  printf("%4d ",sp->st_uid);
  printf("%8d ",sp->st_size);

  // print time
  strcpy(ftime, ctime(&sp->st_ctime));
  ftime[strlen(ftime)-1] = 0;
  printf("%s  ",ftime);

  // print name
  printf("%s", basename(fname));  

  // print -> linkname if it's a symbolic file
  if ((sp->st_mode & 0xF000)== 0xA000){ // YOU FINISH THIS PART
     // use readlink() SYSCALL to read the linkname
     // printf(" -> %s", linkname);
  }
  printf("\n");
}

int ls_dir(char *dname)
{	
  char path[1000];
    strcpy(path,dname);
    DIR *dp;
    struct dirent *files;
    /*structure for storing inode numbers and files in dir
    struct dirent
    {
        ino_t d_ino;
        char d_name[NAME_MAX+1]
    }
    */
    if((dp=opendir(path))==NULL)
        perror("dir\n");
    char newp[1000];
    struct stat buf;
    while((files=readdir(dp))!=NULL)
    {
	if(!strcmp(files->d_name,".") || !strcmp(files->d_name,".."))
		continue;

        //strcpy(newp,path);
        //strcat(newp,"/");
        strcpy(newp,files->d_name); 
        //printf("%s\n",newp);
		
	ls_file(newp);

            //stat function return a structure of information about the file    
        if(stat(newp,&buf)==-1)
        	perror("stat");

    }
	
	
}



