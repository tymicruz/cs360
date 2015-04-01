#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>//errno strerror(errno)
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>

#define  MAX 256

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int  sock, newsock;                  // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;                   // help variablesi
int fd;
int pid = 0, status = 0;
FILE *fp;
char cwd[MAX];//current working directory
char hd[MAX];//homedirectory

int getRequest(int myargc, char* myargv[]);
int putRequest(int myargc, char* myargv[]);
int doCmd (int argc, char* argv[]);
char *getHomeDir(char *env[]);
int ls_dir(char *dname);
int ls_file(char *fname);

// Server initialization code:

int server_init(char *name)
{
   printf("==================== server init ======================\n");   
   // get DOT name and IP address of this host

   printf("1 : get and show server host info\n");
   hp = gethostbyname(name);
   if (hp == 0){
      printf("unknown host\n");
      exit(1);
   }
   printf("    hostname=%s  IP=%s\n",
              (char*)hp->h_name,  (char*)inet_ntoa(*(long *)hp->h_addr));
  
   //  create a TCP socket by socket() syscall
   printf("2 : create a socket\n");
   sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock < 0){
      printf("socket call failed\n");
      exit(2);
   }

   printf("3 : fill server_addr with host IP and PORT# info\n");
   // initialize the server_addr structure
   server_addr.sin_family = AF_INET;                  // for TCP/IP
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address  
   server_addr.sin_port = 0;   // let kernel assign port

   printf("4 : bind socket to host info\n");
   // bind syscall: bind the socket to server_addr info
   r = bind(sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
   if (r < 0){
       printf("bind failed\n");
       exit(3);
   }

   printf("5 : find out Kernel assigned PORT# and show it\n");
   // find out socket port number (assigned by kernel)
   length = sizeof(name_addr);
   r = getsockname(sock, (struct sockaddr *)&name_addr, &length);
   if (r < 0){
      printf("get socketname error\n");
      exit(4);
   }

   // show port number
   serverPort = ntohs(name_addr.sin_port);   // convert to host ushort
   printf("    Port=%d\n", serverPort);

   // listen at port with a max. queue of 5 (waiting clients) 
   printf("5 : server is listening ....\n");
   listen(sock, 5);
   printf("===================== init done =======================\n");
}


main(int argc, char *argv[], char* env[])
{
   char *hostname;
   char line[MAX];
	char linecopy[MAX];
	char token[32][64];
	char* word;
	char* cmd;
	char *myargv[32];
	int myargc = 0, i = 0;
	bzero(cwd, MAX);

	strcpy(hd,getHomeDir(env));

   if (argc < 2)
      hostname = "localhost";
   else
      hostname = argv[1];
 
   server_init(hostname); 

   // Try to accept a client request
   while(1){
     printf("server: accepting new connection ....\n"); 

     // Try to accept a client connection as descriptor newsock
     length = sizeof(client_addr);
     newsock = accept(sock, (struct sockaddr *)&client_addr, &length);
    printf("%d\n", newsock);
	 if (newsock < 0){
        printf("server: accept error\n");
        exit(1);
     }
     printf("server: accepted a client connection from\n");
     printf("-----------------------------------------------\n");
     printf("        IP=%s  port=%d\n", (char*)inet_ntoa(client_addr.sin_addr.s_addr),
                                        ntohs(client_addr.sin_port));
     printf("-----------------------------------------------\n");

     // Processing loop
     while(1){
       n = read(newsock, line, MAX);
		 printf("%d things\n", n);
       if (n==0){
           printf("server: client died, server loops\n");
           close(newsock);
           break;
      }
      
      // show the line string
      printf("server: read  n=%d bytes; line=[%s]\n", n, line);
		strcpy(linecopy, line);
		//-----tokenize string
		word = strtok(linecopy, " ");
		i = 0;
		myargc = 0;

		while(word)
		{
			myargc++;
			strcpy(token[i++], word);
			word = strtok(NULL, " ");
		}

		myargc = i;

		for(i = 0; i < myargc; i++)
		{
			myargv[i] = token[i];
		}

		myargv[i] = 0;//null terminate
		cmd = myargv[0];
		//------tokenize string

		doCmd(myargc, myargv);
		/**if(strcmp(cmd, "get") == 0)
		{
			getRequest(myargc, myargv);
		}else if(strcmp(cmd, "put") == 0)
		{
			putRequest(myargc, myargv);
		}else if(strcmp(cmd, "pwd")==0){
		
		}else{
			strcat(line, " didn't do that");
			n = write(newsock, line, MAX);
		}**/

	printf("\n");	
      //strcat(line, " ECHO");

		
      // send the echo line to client 
      //n = write(newsock, line, MAX);

		//write(newsock, "1", 2);	
      //printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
      //printf("server: ready for next request\n");
    }
 }
}

int getRequest(int myargc, char* myargv[])
{
	int m = 0, i, r;//local r
	char buf[1024];
	struct stat fstat, *sp;
	bzero(buf, 1024);
	
	sp = &fstat;
	
			if(myargc > 1)
			{
				if(lstat(myargv[1], &fstat) < 0)
				{
					write(newsock, "0", 2);//tell client to go away
					printf("can't lstat file\n");
					return 1;
				}
				else
				{
					if((sp->st_mode & 0xF000) == 0x4000)//can't 
					{
						printf("not going to give a dir\n");
						write(newsock, "0", 2);//tell client to go away
						return 0;
					}
				}
				
				//tell clients it's all good
				fd = open(myargv[1], O_RDONLY);

				if(fd < 0)
				{
					printf("file open fail\n");
					write(newsock, "0", 2);//fail
				}
					
				write(newsock, "1", 2);//sucess
				printf("putting file:%s to client\n", myargv[1]);	
				while(m = read(fd, buf, 1024))//read file
				{
					//printf("%d\n", m);
					//write to client
					write(newsock, buf, 1024);//right to sock

					if(m >= 1024)
					{
						write(newsock, "1", 2);//let client know, more to come
					}
					else
					{
						write(newsock, "0", 2);//let client know done
					}
				}

				//write(newsock, "", 1);
				close(fd);
		
			}
			else
			{
				write(newsock, "0", 2);
		
				//tell client fail
			}
}

int putRequest(int myargc, char* myargv[])
{
	char good[2];
	int m = 0, i = 0;
	char buf[1024];
	bzero(buf, 1024);
	//tell client to goahead and start putting out
	write(newsock, "1", 2);

	m = read(newsock, good, 2);

	if(strcmp(good, "0") == 0){return 0;}

	fp = fopen(myargv[1], "w");//open a file

	while((m = read(newsock, buf, 1024)))
	{
		for(i=0; i<m; i++)
		{
			if(buf[i] == 0 || buf[i] == EOF)
			{
				bzero(buf, 1024);
				i = n;
				continue;
				break;
			}else
			{
				putchar(buf[i]);
				fputc(buf[i], fp);
			}
		}
		read(newsock, good, 2);//see if done

		if(strcmp(good, "0") == 0)
		{
			break;
		}

	}
	printf("done - client out it on you!\n");
	fclose(fp);

}

int doCmd (int myargc, char* myargv[])//command will not b null when called
{
	char answer[MAX];
	struct stat fstat, *sp;

	//each command must send a write(newsock, "1" 2); signal to tell client to start listening
	//function will only be called for local commands
	char *cmd = myargv[0];
	//cmd = removeL(cmd);

	char *op = 0;
	sp = &fstat;
	bzero(answer, MAX);

	if(myargc > 1)
	{
		op = myargv[1];
	}//otherwise op is 0
	
	if(strcmp(cmd, "pwd") == 0)
	{
		write(newsock, "1", 2);//tell server, "we good" so it will start listening
		getcwd(cwd, MAX);
		write(newsock, cwd, MAX);
		printf("client request pwd: %s\n", cwd);
		return 1;
	}	
	if(strcmp(cmd, "ls") == 0)
	{
		write(newsock, "1", 2);//tell client to start listening
		if((myargc > 1) && lstat(myargv[1], &fstat) < 0)
		{	
			write(newsock, "0", 2);
			printf("I couldn't lstat %s\n", myargv[1]);
			//sprintf(answer, "I couldn't lstat %s\n", myargv[1]);
			return 0;
		}else if(myargc > 1)
		{
			if(S_ISDIR(sp->st_mode))
			{
				ls_file(myargv[1]);
				write(newsock, "0", 2);//tell client you are done
				return 0;
			}else{
				ls_file(myargv[1]);
				write(newsock, "0", 2);//tell client you are done
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
		write(newsock, "1", 2);//tell client to start listening
		if(!(chdir(op) < 0))//if argv available and chdir success
		{
			//write(newsock, "1", 2);
			printf("client request: cd OK\n");
			strcpy(answer, "cd OK");
			write(newsock, answer, MAX);
		}else if(myargc == 1)
		{
			//write(newsock, "1", 2);
			printf("client cd request to nothing: cd to HOME dir\n");
			chdir(hd);
			//printf("client request: cd OK\n");
			strcpy(answer, "cd to HOME dir");
			write(newsock, answer, MAX);
		}
		else
		{
			//write(newsock, "1", 2);
			printf("client request: cd FAILED\n");
			strcpy(answer, "cd FAILED");
			write(newsock, answer, MAX);
		}
		return 1;
	}	
	if(strcmp(cmd, "mkdir") == 0)
	{
		write(newsock, "1", 2);//tell client to start listening
		if(mkdir(op, 0777) < 0)
		{
			printf("mkdir FAIL -> errno=%d : %s\n", errno, strerror(errno));
			sprintf(answer, "mkdir FAIL -> errno=%d : %s", errno, strerror(errno));
			write(newsock, answer, MAX);
			
		}else
		{
			printf("mkdir %s OK\n", op);
			sprintf(answer, "mkdir %s OK", op);
			write(newsock, answer, MAX);
		}
		return 1;
	}	
	if(strcmp(cmd, "rmdir") == 0)
	{
		write(newsock, "1", 2);//tell client to start listening
		if(rmdir(op) < 0)
		{
			
			printf("rmdir FAIL -> errno=%d : %s\n", errno, strerror(errno));
			sprintf(answer, "rmdir FAIL -> errno=%d : %s", errno, strerror(errno));
			write(newsock, answer, MAX);
			
			//
		}else
		{
			printf("rmdir %s OK\n", op);
			sprintf(answer, "rmdir %s OK", op);
			write(newsock, answer, MAX);

			//
		}
		return 1;
	}	
	if(strcmp(cmd, "rm") == 0)
	{
		write(newsock, "1", 2);//tell client to start listening 
		if(unlink(op) < 0)
		{
			printf("rm FAIL -> errno=%d : %s\n", errno, strerror(errno));
			sprintf(answer, "rm FAIL -> errno=%d : %s", errno, strerror(errno));
			write(newsock, answer, MAX);

		}
		else
		{
			printf("rm %s OK\n", op);
			sprintf(answer, "rm %s OK", op);
			write(newsock, answer, MAX);
		}
		return 1;
	}	
	if(strcmp(cmd, "get") == 0)
	{
		getRequest(myargc, myargv);
		return 2;//need to work with server
	}	
	if(strcmp(cmd, "put") == 0)
	{
		putRequest(myargc, myargv);		
		return 2;
	}	
	if(strcmp(cmd, "cat") == 0)
	{
		write(newsock, "1", 2);//without this client with be stuck
		printf("I don't cat\n");
		sprintf(answer, "I don't cat");
		write(newsock, answer, MAX);
		//tell client you don't cat
	}
	if(strcmp(cmd, "quit") == 0)//should never get this
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

int ls_file(char *fname)
{
  struct stat fstat, *sp;
  int r, i;
  char ftime[64];
char answer[MAX];
char ans[MAX];
bzero(answer, MAX);
bzero(ans, MAX); 
strcpy(ans, "");

  sp = &fstat;
  //printf("name=%s\n", fname); getchar();

  if ( (r = lstat(fname, &fstat)) < 0){
     printf("can't stat %s\n", fname);
 	return 0;
     //exit(1);
  }
	write(newsock, "1", 2);//tell client to listen

  if ((sp->st_mode & 0xF000) == 0x8000){
     printf("%c",'-');
     sprintf(answer, "%c",'-');
	strcat(ans, answer);
	}
  if ((sp->st_mode & 0xF000) == 0x4000){
     printf("%c",'d');
     sprintf(answer, "%c",'d');
	strcat(ans, answer);
	}
  if ((sp->st_mode & 0xF000) == 0xA000){
     printf("%c",'l');
     sprintf(answer, "%c",'l');
	strcat(ans, answer);
	}

  for (i=8; i >= 0; i--){
    if (sp->st_mode & (1 << i))
	{
	printf("%c", t1[i]);
	sprintf(answer, "%c", t1[i]);
	strcat(ans, answer);
	}
    else{
	printf("%c", t2[i]);
	sprintf(answer, "%c", t2[i]);
	strcat(ans, answer);
	}
  }

  printf("%4d ",sp->st_nlink);
  sprintf(answer, "%4d ",sp->st_nlink);
	strcat(ans, answer);
  printf("%4d ",sp->st_gid);
  sprintf(answer,"%4d ",sp->st_gid);
	strcat(ans, answer);
  printf("%4d ",sp->st_uid);
  sprintf(answer, "%4d ",sp->st_uid);
	strcat(ans, answer);
  printf("%8d ",sp->st_size);
  sprintf(answer, "%8d ",sp->st_size);
	strcat(ans, answer);

  // print time
  strcpy(ftime, ctime(&sp->st_ctime));
  ftime[strlen(ftime)-1] = 0;
  printf("%s  ",ftime);
sprintf(answer, "%s  ",ftime);
	strcat(ans, answer);	
  // print name
  printf("%s", basename(fname));  
  sprintf(answer,"%s", basename(fname));
strcat(ans, answer);
  

  // print -> linkname if it's a symbolic file
  if ((sp->st_mode & 0xF000)== 0xA000){ // YOU FINISH THIS PART
     // use readlink() SYSCALL to read the linkname
     // printf(" -> %s", linkname);
  }
  printf("\n");
	write(newsock, ans, MAX);
}

int ls_dir(char *dname)
{	
  char path[1000];
char answer[MAX];
bzero(answer, MAX);
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
	
	write(newsock, "0", 2);//tell client you are done	
}



