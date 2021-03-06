/******* ialloc.c: allocate a free INODE, return its inode number ******/
#ifndef HH
#define HH

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>	
#include <linux/ext2_fs.h>

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

#define BLKSIZE 1024
#define MAX_PATH_LEN 256
#define MAX_PATH_PIECES 64

// Block number of EXT2 FS on FD
#define SUPERBLOCK        1
#define GDBLOCK           2
#define ROOT_INODE        2

// Default dir and regulsr file modes
#define DIR_MODE    0040777 
#define FILE_MODE   0100644
#define SUPER_MAGIC  0xEF53
#define SUPER_USER        0

// Proc status
#define FREE              0
#define READY             1
#define RUNNING           2

// Table sizes
#define NMINODES        100
#define NMOUNT           10
#define NPROC            10
#define NFD              10
#define NOFT            100

// Open File Table
typedef struct oft{
  int   mode;
  int   refCount;
  struct minode *inodeptr;
  int   offset;
}OFT;

// PROC structure
typedef struct proc{
  struct proc *next;  // for link list of PROCs
  int   uid;
  int   pid, gid;
  int   status;
  struct minode *cwd;
  OFT   *fd[NFD];
}PROC;
      
// In-memory inodes structure
typedef struct minode{		
  INODE INODE;               // disk inode
  int   dev, ino;
  int   refCount;
  int   dirty;
  int   mounted;
  struct mount *mountptr;
}MINODE;

// Mount Table structure
typedef struct mount{
        int    dev;
        int    nblocks,ninodes;
        int    bmap, imap, iblk;
        MINODE *mounted_inode;
        char   name[64]; 
        char   mount_name[64];
}MOUNT;


GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 

int fd;
char *disk = "adisk";

//buffers to hold various structures in memory
char super_buf[BLKSIZE];
char group_buf[BLKSIZE];
char inode_buf[BLKSIZE];
char  data_buf[BLKSIZE];

//get these from group descriptor at block 2
int imap_block_G;
int bmap_block_G;
int inode_begin_block_G;

//get from super block
int inodes_per_block_G;
int num_inode_blocks_G;
int num_inodes_G;
int num_blocks_G;

PROC *running, proc[NPROC];//p0, p1, 
MINODE minode[NMINODES], *root;
OFT    oft[NOFT];

//used for printing file mode
char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int get_block(int dev, int blk, char buf[ ]);
int put_block(int dev, int blk, char buf[ ]);
int tst_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
int clr_bit(char *buf, int bit);
int getInodeBlockNumberAndOffset(int inode, int *block_num, int *offset);
int superCheck(int dev);
int getGDInfo(int dev);
int init();
int getino(int dev, char *pathname);
MINODE* iget(int dev, int ino);
int iput (MINODE *mip);
int findmyname(MINODE *parent, int myino, char *myname);
int findino(MINODE *mip, int *myino, int *parentino);
int mount_root();
int list_file(MINODE *mip, char *name);
int list_dir(MINODE *mip);
int ls(char *pathname);
int ls_wrap(char *path);
int cd(char *path);
int cd_wrap(char *path);
char* pwd();
int tokenize(char *path, char *pieces[], int *npieces);
int tokCmd(char *line, char *myargv[], int *myargc);
int nextDataBlock(INODE *ip, int num);
int incFreeInodes(int dev);
int decFreeInodes(int dev);
int ialloc(int dev);
int idealloc(int dev, int block);
int imap(int fd);
int incFreeBlocks(int dev);
int decFreeBlocks(int dev);
int balloc(int dev);
int bdealloc(int dev, int block);
int bmap(int fd);
int quit();

int make_dir(char *npath);
int mymakedir(MINODE *pip, char *name);
int mkdir_wrap(char *path);

int ilen(DIR *d);
int enter_name(MINODE *pip, int myino, char *myname);
int newDataBlock(MINODE *pip);
int nameExists(MINODE *parent, char *myname);

int creat_wrap(char *path);
int creato(char *npath);
int mycreat(MINODE *pip, char *name);

//need to add a dev field to all the my stuff because pid-dev doesn't alwayys = a dir entry's dev

int remove_dir(char *npath);
int myremovedir(MINODE *pip, int ino, char *myname);
int rmdir_wrap(char *path);
int emptyDir(MINODE *pip, MINODE *mip);
int rm_child(MINODE *pip, int myino, char *myname);

//shouldn't be able to link acros devices
//if dev changes when i do the cd behind scenes 
//cd back to og and say can link across

int unlink(char *npath);
int myunlink(MINODE *pip, int ino, char *myname);
int unlink_wrap(char *path);

int link_wrap(char *path1, char *path2, int symflag);
int link(char *path1, char *path2, int symflag);
int mylink(MINODE *pip, int myino, char *myname);
int mysymlink(MINODE *pip, char *oldname, char *name);

int open_file_wrap(char *path, char *mode);
int open_file(char *path, int mode);
int myopen_file(MINODE *mip, int mode);

int truncateInode(MINODE* mip);
OFT* getOFT();

int close_file_wrap(char *fde);
int close_file(int fdesc);
int myclose_file(int fdesc);

int seek_fd_wrap(char *fde, char *stringseek);
int mylseek(int fdesc, int position);
int pfd();

int dup_fd_wrap(char *fde);
int dup_fd(int fdesc);
int mydup_fd(int fdesc);
int dup2_fd_wrap(char *fde, char *gde);
int dup2_fd(int fdesc, int gdesc);
int mydup2_fd(int fdesc, int gdesc);

int read_file_wrap(char *fde, char *stringbytes);
int read_file(int fdesc, int bytes);
int myread_file(int fdesc, int bytes);

int cat_file_wrap(char *path);
int cat_file(int fdesc);
int mycat_file(int fdesc);

int write_file_wrap(char *fde, char *stuff);
int write_file(int fdesc, char *stuff);
int mywrite_file(int fdesc, char stuff[]);

int copy_file_wrap(char *source, char *dest);
int copy_file(int fdsource, int fddest);
int mycopy_file(int fdsource, int fddest);


int get_block(int dev, int blk, char buf[ ]){

	lseek(dev, (long)blk*BLKSIZE, 0);
	read(dev, buf, BLKSIZE);
}

int put_block(int dev, int blk, char buf[ ]){

	lseek(dev, (long)blk*BLKSIZE, 0);
	write(dev, buf, BLKSIZE);
}

int tst_bit(char *buf, int bit){
	//'8' would be 32 if array blocks were 4 bytes (int)
	//e.g. that number will be 8x(size of datatype of buffer in bytes)
	//variable name of 'byte'wouldn't be politically correct if buf's base datatype wasn't 1 byte

	int byte = bit/8;
	int bit_in_byte = bit%8;

	if(buf[byte] & (1 << bit_in_byte))
	{
		return 1;
	}	

	return 0;
}

int set_bit(char *buf, int bit){
	int byte = bit/8;
	int bit_int_byte = bit%8;

	//bit is already set
	if(tst_bit(buf, bit)==1)
	{
		return -1;
	}

	buf[byte] = buf[byte] | (1 << bit_int_byte);
	
	//return set bit
	return bit;
} 

int clr_bit(char *buf, int bit){
	int byte = bit/8;
	int bit_int_byte = bit%8;

	//bit is already clr
	if(tst_bit(buf, bit)==0)
	{
		//return to prevent setting 0 bit
		return -1;
	}
	//xor the bit you want to clr
	//if target bit is 1 it will set to 0
	//if target bit is 0 it will be set to 1
	//untargeted bits:	if 1, bit will stay 1 (1 xor 0 is 1)
	//			if 0, bit will stay 0 (0 xor 0 is 0)
	//xor targeted bit with binary 1
	//we make sure target bit isn't already 0 before we
	//get here, otherwise, instead of clearing bit, we
	//will be setting it
	buf[byte] = buf[byte] ^ (1 << bit_int_byte);
	//return cleared bit
	return bit;
} 

int decFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, SUPERBLOCK, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, SUPERBLOCK, buf);

  get_block(dev, GDBLOCK, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, GDBLOCK, buf);
}

int incFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, SUPERBLOCK, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, SUPERBLOCK, buf);

  get_block(dev, GDBLOCK, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, GDBLOCK, buf);
}

int ialloc(int dev)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap_block_G, buf);

  for (i=0; i < num_inodes_G; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeInodes(dev);

       put_block(dev, imap_block_G, buf);

       return i+1;
    }
  }

  printf("ialloc(): no more free inodes\n");
  return -1;
}

int idealloc(int dev, int ino)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap_block_G, buf);

  if(ino <= num_inodes_G && ino > 10)//don't mess with first 10 inodes
  {
  	i = clr_bit(buf, ino - 1);

  	if(i < 0)
  	{
  		printf("error: ino ibit, %d, is already 0.\n", ino);
  		return -1;
  	}

  	put_block(dev, imap_block_G, buf);
  	incFreeInodes(dev);

  	return ino;
  }
  else
  {
  	printf("can't delloc ino ibit, %d\n", ino);
  	return -1;
  }
}

imap(int fd)
{
  char buf[BLKSIZE];
  int  imap, ninodes;
  int  i;

  // read SUPER block
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  ninodes = sp->s_inodes_count;
  printf("ninodes = %d\n", ninodes);

  // read Group Descriptor 0
  get_block(fd, 2, buf);
  gp = (GD *)buf;

  imap = gp->bg_inode_bitmap;
  printf("inodebitmap = %d\n", imap);

  // read inode_bitmap block
  get_block(fd, imap, buf);

  for (i=0; i < ninodes; i++){
    (tst_bit(buf, i)) ?	putchar('1') : putchar('0');
    if (i && (i % 8)==0)
       printf(" ");
  }
  printf("\n");
}

int decFreeBlocks(int dev)
{
  char buf[BLKSIZE];

  // dec free data blocks count in SUPER and GD blocks
  get_block(dev, SUPERBLOCK, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, SUPERBLOCK, buf);

  get_block(dev, GDBLOCK, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, GDBLOCK, buf);
}

int incFreeBlocks(int dev)
{
  char buf[BLKSIZE];

  // dec free data blocks count in SUPER and GD blocks
  get_block(dev, SUPERBLOCK, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, SUPERBLOCK, buf);

  get_block(dev, GDBLOCK, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, GDBLOCK, buf);
}

int balloc(int dev)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, bmap_block_G, buf);

  for (i=0; i < num_blocks_G; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeBlocks(dev);

       put_block(dev, bmap_block_G, buf);
       return i+1;//return free data block num
    }
  }
  printf("balloc(): no more free data blocks\n");
  return -1;
}

int bdealloc(int dev, int block)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, bmap_block_G, buf);

  if(block <= num_blocks_G && block >= 0)//don't mess with first 10 inodes
  {
  	i = clr_bit(buf, block - 1);

  	if(i < 0)
  	{
  		printf("error: block ibit, %d, is already 0.\n", block);
  		return -1;
  	}

  	put_block(dev, bmap_block_G, buf);
  	incFreeBlocks(dev);

  	return block;
  }
  else
  {
  	printf("can't delloc block bit, %d\n", block);
  	return -1;
  }
}

bmap(int fd)
{
  char buf[BLKSIZE];
  int  bmap, nblocks;
  int  i;

  // read SUPER block
  get_block(fd, SUPERBLOCK, buf);
  sp = (SUPER *)buf;

  nblocks = sp->s_blocks_count;
  printf("nblocks = %d\n", nblocks);

  // read Group Descriptor 0
  get_block(fd, GDBLOCK, buf);
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  printf("blockbitmap = %d\n", bmap);

  // read inode_bitmap block
  get_block(fd, bmap, buf);

  for (i=0; i < nblocks; i++){
    (tst_bit(buf, i)) ?	putchar('1') : putchar('0');
    if (i && (i % 8)==0)
       printf(" ");
  }
  printf("\n");
}

int getInodeBlockNumberAndOffset(int inode, int *block_num, int *offset){
	//adjust one to sync the first inode with 0

	*block_num = ((inode - 1) / inodes_per_block_G) + inode_begin_block_G;
	*offset = (inode - 1) % inodes_per_block_G;
}

//call this in init
int superCheck(int dev)
{
	get_block(dev, SUPERBLOCK, super_buf);//super block is the 1th block
	//buf now holds a SUPER struct

	sp = (SUPER *)super_buf;
	//(SUPER *) is just telling compiler "shut up, I know what I'm doing"
	//(SUPER *) is unnecessary

	//is this an EXT2 filesystem?
	//let's check the magic number
	printf("the SUPER s_magic num of this disk image is 0x%04x.\n", sp->s_magic);
	printf("the magic number for an EXT2 file system is 0xEF53.\n");

	if(sp->s_magic != SUPER_MAGIC)
	{
		printf("NOT an EXT2 file system\n");
		printf("exiting program ..\n");
		exit(1);
	}
	
	printf("Great, they match!\n");
	printf("Therefore, this disk image is an EXT2 file system.\n\n");

	printf("ENTER for super block content:\t\t|\n"); 
	printf("\t\t\t\t\t|\n");
	printf("\t\t|-----------------------|\n");
	printf("\t\t|\n");
	printf("\t\tV\n");

	pressEnterToContinue();

	printf("SUPER BLOCK content at %dth block:\n", SUPERBLOCK);
  	printf("s_inodes_count \t\t= %d\n", sp->s_inodes_count);
  	printf("s_blocks_count \t\t= %d\n", sp->s_blocks_count);

  	printf("s_free_inodes_count \t= %d\n", sp->s_free_inodes_count);
  	printf("s_free_blocks_count \t= %d\n", sp->s_free_blocks_count);
  	printf("s_first_data_blcok \t= %d\n", sp->s_first_data_block);
	
  	printf("s_log_block_size \t= %d\n", sp->s_log_block_size);
  	printf("s_log_frag_size \t= %d\n", sp->s_log_frag_size);

  	printf("s_blocks_per_group \t= %d\n", sp->s_blocks_per_group);
  	printf("s_frags_per_group \t= %d\n", sp->s_frags_per_group);
  	printf("s_inodes_per_group \t= %d\n", sp->s_inodes_per_group);

  	printf("s_mnt_count \t\t= %d\n", sp->s_mnt_count);
  	printf("s_max_mnt_count \t= %d\n", sp->s_max_mnt_count);

  	printf("s_magic \t\t= %x\n", sp->s_magic);
	printf("s_inode_size \t\t= %d\n", sp->s_inode_size);

  	printf("s_mtime \t\t= %s", (char *)ctime(&sp->s_mtime));
  	printf("s_wtime \t\t= %s", (char *)ctime(&sp->s_wtime));

	//find how many inodes there are per block
	inodes_per_block_G = BLKSIZE/(sp->s_inode_size);
	num_inode_blocks_G = (sp->s_inodes_count)/		inodes_per_block_G;


	//if num inodes doesn't fit directly in then you need one more block that wont be use fully
	if((sp->s_inodes_count)%inodes_per_block_G!=0)
	{
		printf("last block of inodes won't be used fully\n");
		num_inode_blocks_G++;
	}

	num_inodes_G = inodes_per_block_G * num_inode_blocks_G;//or = sp->s_inodes_count;
	num_blocks_G = sp->s_blocks_count;
	printf("inodes per block \t= %d\n", inodes_per_block_G);
	printf("num inode blocks \t= %d\n", num_inode_blocks_G);
	printf("num inodes \t\t= %d\n", num_inodes_G);
	printf("num blocks \t\t= %d\n", num_blocks_G);
}

//call this in init
int getGDInfo(int dev)
{
	printf("\nENTER for GD block content:\t\t|\n"); 
	printf("\t\t\t\t\t|\n");
	printf("\t\t|-----------------------|\n");
	printf("\t\t|\n");
	printf("\t\tV\n");
	pressEnterToContinue();
	
	get_block(dev, GDBLOCK, group_buf);
	gp = (GD*)group_buf;
	printf("GD BLOCK content at %dth block:\n", GDBLOCK);

	imap_block_G = gp->bg_inode_bitmap;
	bmap_block_G = gp->bg_block_bitmap;  
	inode_begin_block_G = gp->bg_inode_table;

	get_block(fd, inode_begin_block_G, inode_buf);
	//getting first inode
	ip = (INODE *)inode_buf + 1;//get first inode which is root (really the 2nd inode)

	printf("%-8d = block bitmap block\n%-8d = inode bitmap block\n%-8d = inode begin block\n%-8d = free blocks count\n%-8d = free inodes count\n%-8d = used dirs count\n\n",
	 gp->bg_block_bitmap,
	 gp->bg_inode_bitmap,
	 gp->bg_inode_table,
	 gp->bg_free_blocks_count,
	 gp->bg_free_inodes_count,
	 gp->bg_used_dirs_count);
}

//init is called first then mount root
int init()
{
	int i = 0;

	running = (PROC*)malloc(sizeof(PROC));

	//p0;// = (PROC*)malloc(sizeof(PROC));
	//p1;// = (PROC*)malloc(sizeof(PROC));

	//init proc strucutres
	running = &(proc[0]);

	proc[0].uid = 0;
	proc[0].gid = 0;
	proc[0].status = 1;
	//*(p0.fd) = 0;
	for (i = 0; i < NFD; ++i)
	{
		proc[0].fd[i] = 0;
	}

	proc[1].uid = 1;
	proc[1].gid = 1;
	proc[1].status = 0;
	//*(p1.fd) = 0;
	for (i = 0; i < NFD; ++i)
	{
		proc[1].fd[i] = 0;
	}

	//init all minodes
	for(i = 0; i < NMINODES; i++)
	{	
		//printf("%d\n",i);
		minode[i].refCount=0;
		minode[i].dev=0;
		minode[i].ino=0;
		minode[i].dirty=0;
		minode[i].mounted=0;
		minode[i].mountptr = 0;
	}

  for(i = 0; i < NOFT;i++)
  {
    oft[i].mode = 0;
    oft[i].refCount = 0;
    oft[i].inodeptr = 0;
    oft[i].offset = 0;
  }

	root = 0;
}

//mount root called right after init
int mount_root()
{
	int me, parent;
	char myname[32] = {0};

	
	fd = open(disk, O_RDWR);

	if(fd < 0)
	{
		printf("couldn't open %s\n", disk);
		exit(1);
	}

	superCheck(fd);
	getGDInfo(fd);

	root = iget(fd, ROOT_INODE);
 	proc[0].cwd = iget(fd, ROOT_INODE); 
 	proc[1].cwd = iget(fd, ROOT_INODE);

	printf("root minode refCount = %d\n", root->refCount);

	running->cwd = iget(fd, ROOT_INODE);
}


//only goes up to doubley indirect blocks
//this takes a full path from root
int getino(int dev, char *pathname)
{
	char *path_pieces[MAX_PATH_PIECES];
	//char current_directory[MAX_PATH_LEN] = "/";
	int path_count = 0, search_count = 0;
	int block_num = 0, offset = 0;

	int found = 0, no_hope = 0;
	char *cp, temp = 0;
	int i = 0;
	int more = 0;
	int block_check = 0;

	MINODE *mip;
	
	//printf("%s\n", pathname);
	tokenize(pathname, path_pieces, &path_count);

	if(path_count == 0)
	{
		return ROOT_INODE;
	}

	//load first inode block into memory so we can get root
	//get_block(fd, inode_begin_block_G, inode_buf);
	mip = iget(dev, 2);

	//ip = (INODE *)inode_buf + 1;//get first inode which is root
	ip = &(mip->INODE);

	//load data block of root inode
	get_block(fd, ip->i_block[block_check], data_buf);	
	
	dp = (DIR *)data_buf;
	cp = data_buf;//same as cp = data_buf

	more = nextDataBlock(ip, block_check++);

	//checking path pieces because they were jacked up from tokenize
	/*	for(i = 0; i < path_count; i++)
	{
		printf("%d:%s\n",i+1, path_pieces[i]);
	}*/

	while(search_count < path_count && !no_hope && !found && more)
	{
		dp = (DIR *)data_buf;
		cp = data_buf;
		more = nextDataBlock(ip, block_check++);

		while((cp < (data_buf + BLKSIZE)))
		{
			//null terminate the dp->name field since there is no null char
			printf("rec len: %-4d -> ", dp->rec_len);
			temp = dp->name[dp->name_len];
			dp->name[dp->name_len] = 0;
			printf("%-15s -> target(%s)", dp->name, path_pieces[search_count]);//getchar();
			//dir name match //
			if(strcmp(path_pieces[search_count], dp->name) == 0)
			{
				printf("match ");
				search_count++;
				//printf("match:%s, %d more to go\n", dp->name, path_count - search_count);//getchar();
				if(search_count < path_count)//keep looking, not at end of path
				{
					//strcat(current_directory, dp->name);
					//strcat(current_directory, "/");

					dp->name[dp->name_len] = temp;//put char back that we moved
					
					//get inode of this thing we just found
					//getInodeBlockNumberAndOffset(dp->inode, &block_num, &offset);
					
					iput(mip);
					mip = iget(dev, dp->inode);
					//get_block(fd, block_num, inode_buf);
					//now we have inode of current path we've seen so far
					ip = &(mip->INODE);//(INODE *)inode_buf + offset;

					if(!S_ISDIR(ip->i_mode))//cant search anymore because this isn't a dir
					{
						no_hope = 1;
						printf(" X, not directory\n");
						iput(mip);
						//printf("path is at dead end cuz we at a file\n");
						return 0;
						//break out of inner loop and then will break out of outer because no_hope = 1;
						break;
					}
				
					//if match a directory, we go here
					printf("keep looking\n");
					//get first data block of this dir that we just found
					block_check = 0;
					get_block(fd, ip->i_block[block_check], data_buf);
					//get_block(fd, more, data_buf);
					//GET MORE OF THIS SO WE CAN ENTER NEXT LOOP
					more = nextDataBlock(ip, block_check++);
				}
				else
				{
					//printf("found\n");
					//found

					//sometime dp-> is found but dp->inode is still 0

					dp->name[dp->name_len] = temp; //put char back that we moved
					printf("FOUND IT: %d!\n", dp->inode);

					//WE FOUND A MATCH IN THIS DIR
					//NO WE NEED TO GO TO THE INODE FROM WHAT WE SEE AT THE CURRENT DP POINTER
					//if we need to look to get data blocks
					//getInodeBlockNumberAndOffset(dp->inode, &block_num, &offset);
					//get_block(fd, block_num, inode_buf);
					//ip = (INODE *)inode_buf + offset;
					iput(mip);
					return dp->inode;
					found = 1;
				}	
				//WE WILL BREAK IF WE MATCH TO NEXT ITERATION
				break;
			}
			else
			{
				printf("continue ");
				//MUST PUT THE CHAR BACK 	~!!!!!!!!! SUCH A STUPUD BUG
				dp->name[dp->name_len] = temp;
			}

			cp+=(dp->rec_len);
			dp = (DIR *)cp;

			//if we have looked through the end of the dir and we haven't seen the name
			if(cp >= (data_buf + BLKSIZE) && !more)//went through whole dir and we didn't find match
			{
				printf("\nend of buf, and we can't find\n");
				no_hope = 1;
				iput(mip);
				return 0;
				break;
			}
			else if(cp >= (data_buf + BLKSIZE))
			{
				printf("\n");
				//check next data block if you didn't find it in this data block
				get_block(fd, more, data_buf);
				break;
			}
			printf("\n");
		}
	}

	iput(mip);
	//couldn't find dat ish
	return 0;
}

//need to make sure we are getting a node that exists
MINODE* iget(int dev, int ino)
{	
	int block_num = 0, offset = 0, i = 0;

	//inode is not in memory
	for(i = 0; i < NMINODES; i++)
	{
		//found an unused minode
		if(minode[i].ino == ino && minode[i].dev == dev && minode[i].refCount == 0)
		{
			minode[i].ino = ino;
			minode[i].dev = dev;
			minode[i].dirty = 0;
			minode[i].refCount = 1;

			getInodeBlockNumberAndOffset(ino, &block_num, &offset);
			get_block(dev, block_num, inode_buf);
			ip = (INODE*)inode_buf + offset;

			//copy ip node to inmemoryinode	
			memcpy(&(minode[i].INODE), ip, sizeof(INODE));
			//printf("can't find it");getchar();
			return &(minode[i]);
		}
		else if(minode[i].ino == ino && minode[i].dev == dev){
			minode[i].refCount++;

			return &(minode[i]);
		}
	}

	for(i = 0; i < NMINODES; i++)
	{
		//found an unused minode
		if(minode[i].refCount == 0)
		{
			minode[i].ino = ino;
			minode[i].dev = dev;
			minode[i].dirty = 0;
			minode[i].refCount = 1;

			getInodeBlockNumberAndOffset(ino, &block_num, &offset);
			get_block(dev, block_num, inode_buf);
			ip = (INODE*)inode_buf + offset;

			//copy ip node to inmemoryinode	
			memcpy(&(minode[i].INODE), ip, sizeof(INODE));
			//printf("can't find it");getchar();
			return &(minode[i]);
		}
	}
	//RAN OUT OF MINODES FOR ALLOCATING
	return 0;
}

int iput (MINODE *mip)
{
	int block_num = 0, offset = 0, i = 0;

	//decrement ref count
	(mip->refCount)--;

	//if refCount is > 0, others are looking and we don't care
	if((mip->refCount) > 0)
	{
		return;
	}

	//ref count is less than = to 0 at this point

	//in memory inode is same as on disk so we don't need to put anything
	if((mip->dirty) == 0)
	{
		//should already be 0,
		//but if we pass in an MINODE with ref count 0 we could 
		//make refCount Negative, so to be safe we'll set it to 0
		mip->refCount = 0; //make sure refCount is 0 and not negative
		return;
	}
	printf("ino %d is dirty(%d), writing back!\n", mip->ino, mip->dirty);
	//if we get here, ref count is 0 (or negative) 
	//which means we are the last one looking at this inode and it is different than
	//it is on disk, so we need to update the on disk inode

		//
		getInodeBlockNumberAndOffset(mip->ino, &block_num, &offset);
		get_block(mip->dev, block_num, inode_buf);
	
		//load inode from disk
		ip = (INODE*)inode_buf + offset;

		//copy inmemoryinode to ip pointer which points to inode_buf
		memcpy(&(*ip), &(mip->INODE), sizeof(INODE));

		//write inode_buf back to disk
		put_block(fd, block_num, inode_buf);

		//now inode is updated on disk
	//now it is implied that refCount is 0
	//we know refCount must now be 0; otherwise it must be negative which means something else messed up to have an imemory node with a negative refCount
	return;
}

int findmyname(MINODE *parent, int myino, char *myname)
{
	char *cp, temp;
	int more, block_check = 0, i = 0;

	//if passed a null pointer
	if (parent == 0){printf("wtf null mide in fmyname()\n");return 0;}

	//int block_num, offset;
	//idont need to get inode from disk because it is already in memory
	//getInodeBlockNumberAndOffset(parent->ino, &block_num, &offset);
	
		//ip = (INODE *)inode_buf + 1;//get first inode which is root
	//load data block of root inode
	get_block(fd, (parent->INODE).i_block[block_check], data_buf);	
	
	dp = (DIR *)data_buf;
	cp = data_buf;//same as cp = data_buf

	more = nextDataBlock(&(parent->INODE), block_check++);
	
	//make sure we check all data blocks of dir, not just i_block[0]
	while(more)
	{
		more = nextDataBlock(&(parent->INODE), block_check++);

		while(cp < data_buf + BLKSIZE)
		{
			if(dp->inode == myino)
			{
				//we found the child's ino in the dir, so we have access to its name
				for(i = 0; i < dp->name_len; i++)
				{
					myname[i] = dp->name[i];
				}

				myname[i] = 0;//null terminate the string

				//return myname after copying dp->name to myname
				return 1;		
			}	

			//move cp and dp pointers
			cp+= dp->rec_len;
			dp = (DIR*)cp;
		}

		//if there's more get it, else we'll break on the next outer loop
		if(more)
		{
				get_block(parent->dev, more, data_buf);	
	
				dp = (DIR *)data_buf;
				cp = data_buf;//same as cp = data_buf
				//i could break here 
		}
	}

	return 0;
}

int findino(MINODE *mip, int *myino, int *parentino)
{
  char *cp, temp;
	int more, block_check = 0;
	int found = 0;

	//if passed a null pointer
	if (mip == 0){return 0;}

	//int block_num, offset;
	//idont need to get inode from disk because it is already in memory
	//getInodeBlockNumberAndOffset(parent->ino, &block_num, &offset);
	
		//ip = (INODE *)inode_buf + 1;//get first inode which is root
	//load data block of root inode
	get_block(fd, (mip->INODE).i_block[block_check], data_buf);	
	
	dp = (DIR *)data_buf;
	cp = data_buf;//same as cp = data_buf

	more = nextDataBlock(&(mip->INODE), block_check++);
	
	//make sure we check all data blocks of dir, not just i_block[0]
	while(more && found < 2)
	{
		more = nextDataBlock(&(mip->INODE), block_check++);
		

		while((cp < data_buf + BLKSIZE) && found < 2)
		{
			//null terminate dp->name so we can work with it
			temp = dp->name[dp->name_len];
			dp->name[dp->name_len] = 0;
			//printf("%d cp:%d data_buf:%d dp->name_len:%d\n", more, cp, data_buf + BLKSIZE, dp->name_len);
			//printf("%s\n", dp->name); getchar();

			if(strcmp(dp->name, ".") == 0)
			{
				dp->name[dp->name_len] = temp;//adjust dp back to normal
				*myino = dp->inode;
				found++;
			}
			else if(strcmp(dp->name, "..") == 0)
			{
				//printf("%s\n", dp->name); getchar();
				dp->name[dp->name_len] = temp;//adjust dp back to normal
				*parentino = dp->inode;
				found++;
			}
			else
			{
				dp->name[dp->name_len] = temp;//adjust dp back to normal			
			}

			//move cp and dp pointers
			cp+= dp->rec_len;
			dp = (DIR*)cp;
		}

		//if there's more get it, else we'll break on the next outer loop
		if(more && found < 2)
		{
				get_block(fd, more, data_buf);	
	
				dp = (DIR *)data_buf;
				cp = data_buf;//same as cp = data_buf

				//i could break here 
		}
	}

	if(found == 2)
	{
		return 1;
	}

	//both not found
	return 0;
}


//used in ls
int list_file(MINODE *mip, char *name)
{
	char ftime[64];
	INODE *ip = &(mip->INODE);	
  int i;
	//print file mode
  if ((ip->i_mode & 0xF000) == 0x8000)
     printf("%c",'-');
  if ((ip->i_mode & 0xF000) == 0x4000)
     printf("%c",'d');
  if ((ip->i_mode & 0xF000) == 0xA000)
     printf("%c",'l');

	//print permissions
	for (i=8; i >= 0; i--){
		//if bit is 1
    if (tst_bit((char*)&(ip->i_mode), i))
		{
			printf("%c", t1[i]);
		}	
    else
		{
			printf("%c", t2[i]);
		}
  }

  printf("%4d ",ip->i_links_count);
  printf("%4d ",ip->i_gid);
  printf("%4d ",ip->i_uid);
  printf("%8d ",ip->i_size);

  // print time
  strcpy(ftime, (char *)ctime(&ip->i_ctime));
  ftime[strlen(ftime)-1] = 0;
  printf("%s  ",ftime);

  // print name
  printf("%s", name);  


  // print -> linkname if it's a symbolic file
  if ((ip->i_mode & 0xF000)== 0xA000){ // YOU FINISH THIS PART
			printf(" -> %s", (char *)ip->i_block);}
     // use readlink() SYSCALL to read the linkname
     // printf(" -> %s", linkname);
  printf("\n");
}

//used in ls
int list_dir(MINODE *mip)
{
	MINODE *cip;
	char *cp, temp;
	int block_check = 0, more = 0;
	ip = &(mip->INODE);

	more = nextDataBlock(&(mip->INODE), block_check++);

	//make sure we get all data blocks associated with this dir
	while(more)
	{
		get_block(mip->dev, more, data_buf);
		dp = (DIR*)data_buf;
		cp = data_buf;

		while((int)cp < (int)data_buf + BLKSIZE)
		{
			temp = dp->name[dp->name_len];
			dp->name[dp->name_len] = 0;

			//always put when your done getting!!!
			cip = iget(mip->dev, dp->inode);
			list_file(cip, dp->name);
			//iput(cip);
			dp->name[dp->name_len] = temp;
			
			iput(cip);
			cp+=dp->rec_len;
			dp=(DIR*)cp;
			//printf("cp->%d, data_buf + bs->%d", cp, data_buf + BLKSIZE);
		}
		
		more = nextDataBlock(&(mip->INODE), block_check++);
		
	}
}

//used in ls wrap
int ls(char *pathname)
{
	int ino = ROOT_INODE; 
	MINODE *mip;
	//printf("ls%s\n", pathname);
	//if pathname is not root, get ino
	if(strcmp(pathname, "/") != 0)
	{
		//printf("lsing from root\n");
		//printf("path: %s\n", pathname);
		ino = getino(fd, pathname);
		//printf("%d\n", ino);//getchar();
	}

	if(!ino)
	{
		printf("ls: can't find that garbage\n");
		return;
	}

	mip = iget(fd, ino);

	if(!S_ISDIR(mip->INODE.i_mode))
	{
		list_file(mip, (char *)basename(pathname));
	}
	else
	{
		list_dir(mip);
	}

	iput(mip);
}

//ls wraps makes sure to give ls a correct/useable 
//and path name
int ls_wrap(char *path)
{
	char lspath[MAX_PATH_LEN] = "";

	if(path == 0)
	{
		strcpy(lspath, pwd());
		printf("*ls current dir*\n");
		ls(lspath);
		return;
	}

	//we'll ls from root
	if(path[0] == '/')
	{
		ls(path);
		return;
	}

	//go from current dir
	strcat(lspath, pwd());
	strcat(lspath, "/");
	strcat(lspath, path);

	//printf("try to ls this:%s\n", lspath);

	ls(lspath);
}

int cd(char *path)
{
	MINODE *mip;
	int ino;
			
	if(0 == path)
	{
		mip = iget(fd, 2);
		iput(running->cwd);
		printf("*cd to root*\n");
		running->cwd = mip;
		return 1;
		//cd to rood
	}

	ino = getino(fd, path);

	//if ino = 0 return, we d
	if(!ino) 
	{
		printf("can't cd to that garbage\n");
		return -1;
	}

	mip = iget(fd, ino);

	//if inode doesn't exit
	if(!S_ISDIR(mip->INODE.i_mode))
	{
		iput(mip);
		printf("can't cd to a non-directory file\n");
		return -1;
	}
	
	//file is a directory at this point

	//iput minode of running proc
	//put current wd inode back and point to new one
	iput(running->cwd);
	//set running cwd to in memory inode that we just got 
	running->cwd = mip;
	printf("cd to %s success\n", pwd());
	return 1;
//printf("cd succes\n");
}

//makes sure to give cd correct input
int cd_wrap(char *path)
{
	char cdpath[MAX_PATH_LEN] = "";
	int success = 0;

	if(path == 0)
	{
		return cd(0);//change dir to root
		return;
	}
	//we'll ls from root
	if(path[0] == '/')
	{
		return cd(path);
		return;
	}

	//go from current dir
	strcat(cdpath, pwd());
	strcat(cdpath, "/");
	strcat(cdpath, path);

	//change dir from current working directory

	//printf("try to cd to this:%s\n", cdpath);

	return cd(cdpath);
}
//char cwd[256];

char* pwd()
{
	int myino, parentino;
	char rpath[256] = "";
	char *path;//path[256] = "";
	char *cp;
	char myname[64];
	
	MINODE *mip = running->cwd;
	MINODE *parentmip;// = iget(mip->dev, mip->ino);//parent mipinode

	//get inode of parent from .. until we get to inode 2
	//then we end path with /
	findino(mip, &myino, &parentino);

	if(myino == root->ino)
	{
		return "/";
	}
	//need to check if findino was successful
	parentmip = iget(mip->dev, parentino);
	findmyname(parentmip, myino, myname);
	//printf("-->here");getchar();
	
	while(myino != root->ino)//or while(myino != parentino)
	{
		//printf("%s->%d-%d", myname, myino, parentino);
		//getchar();
		strcat(rpath, "/");
		strcat(rpath, myname);

		findino(parentmip, &myino, &parentino);
		iput(parentmip);//put it away
		parentmip = iget(mip->dev, parentino);
		findmyname(parentmip, myino, myname);
	}

	iput(parentmip);//put it away
	//printf("-->%s<--\n", rpath);

	cp = (char*)basename(rpath);
	path = (char *)malloc(sizeof(char) * (strlen(rpath) + 1));
	path[0] = 0;

	//comment this i dont know what i was doing here
	//reverseing path
	//i'm reversing path and storing in another char array
	while(strlen(cp))//until cp is blank
	{
		strcat(path, "/");
		strcat((char*)path, cp);

		//then chop rpath so we can get a new base path
		//note that r path is originally the backwards path of rpath
		rpath[strlen((char*)rpath) - strlen((char*)basename(rpath)) - 1] = 0;
		cp = (char*)basename(rpath);
	}
	//printf("-->%s<--\n", path);
	return (char*)path;
}

int mkdir_wrap(char *path)
{
	if(path == 0)
	{
		printf("give something to make\n");
		return -1;
	}

	while(strlen(path) > 1 && path[strlen(path) - 1] == '/')
	{
		//take off excess '/'
		printf("%s\n", path);
		path[strlen(path) - 1] = 0;
	}

	make_dir(path);
}

int make_dir(char *npath)
{
	char *ogpath = 0, parent[MAX_PATH_LEN], child[MAX_PATH_LEN];
	int check = 0;

	MINODE *pip;// = running->cwd;

	//printf("herex-->1");getchar();
	ogpath = (char *)malloc(sizeof(char) * (strlen(pwd() + 1) + 1));
	strcpy(ogpath, pwd());
	//printf("%s\n", ogpath);

	//printf("herex-->2");getchar();
	//printf("p:%s\n", npath);
	strcpy(child, (char*)basename(npath));
	//printf("child:%s\n", child);

	if(strlen(child) == 0)
	{
		printf("houston we have a problem, can't make dir here\n");
		return -1;
	}

	//printf("herex-->2");getchar();
	//printf("p:%s\n", npath);
	strcpy(parent, (char*)dirname(npath));
	//printf("prnt:%s\n", parent);


	//change dir to parent dir
	check = cd_wrap(parent);

	if(check < 0)
	{
		printf("can't make this dir, bad path\n");
		return -1;
	}

	//
	pip = running->cwd;

	//try to make dir in parent directory, may fail if it exists or not enough data or inodes
	mymakedir(pip, child);

	//goback to original working directory
	cd_wrap(ogpath);
}

int mymakedir(MINODE *pip, char *name)
{
	int i, ino, bno;
	char *cp;
	MINODE *mip;

	if(pip == 0) 
	{
		printf("wtf parent mip is null\n");
		return -1;
	}

	ino = ialloc(pip->dev);
	bno = balloc(pip->dev);

	if(ino < 0 || bno < 0)
	{
		printf("no room for dir: ino %d, bno %d\n", ino, bno);
		return -1;
	}

	//make sure name doesn't exist already
	if(nameExists(pip, name))
	{
		idealloc(pip->dev, ino);
		bdealloc(pip->dev, ino);
		printf("dir already exists\n");
		return -1;
	}

	//get this ino from memory
	//this ino will be blank
	//we need to set all of it's attributes
	mip = iget(pip->dev, ino);

	//now we have to make this inode a dir
	ip = &(mip->INODE);
	
	ip->i_mode = 0x41ED; //or 040755
	ip->i_uid = running->uid;
	ip->i_gid = running->gid;
	ip->i_size = BLKSIZE;
	ip->i_links_count = 2;
	ip->i_atime = time(0);
	ip->i_ctime = time(0);
	ip->i_mtime = time(0);
	ip->i_blocks = 2;//linux blocks count in 512-byte chunks
	ip->i_block[0] = bno;

	for(i = 1; i <= 14; i++)
	{
		ip->i_block[i] = 0;
	}

	mip->dirty = 1;//mark bit as dirty so it gets written back to disk;
	printf("new ino will be %d\n", ino);
	iput(mip);

	//write . and .. to data block
	//bzero(data_buf, BLKSIZE);
	get_block(pip->dev, bno, data_buf);

	//zero out the buf array
	bzero(data_buf, BLKSIZE);

	//increment pip links count
	pip->INODE.i_links_count++;
	pip->dirty = 1;


	dp = (DIR *)data_buf;
	cp = data_buf;

	//write .
	dp->inode = mip->ino;
	dp->rec_len = 12;
	dp->name_len = 1;
	//dp->file_type = 0;
	dp->name[0] = '.';

	//increment dp
	cp += dp->rec_len;
	dp = (DIR*)cp;

		//write ..
	dp->inode = pip->ino;
	dp->rec_len = BLKSIZE - 12;//give 2nd dir entry the rest
	dp->name_len = 2;
	//dp->file_type = 0;
	dp->name[0] = '.';
	dp->name[1] = '.';

	//write buf to disk
	//i pray that this data_buf is written to correctly, before it goes to disk
	put_block(pip->dev, bno, data_buf);
	
	enter_name(pip, ino, name);
}

int enter_name(MINODE *pip, int myino, char *myname)
{
	int block_check = 0, i = 0;
	int more = 0, ideal = 0, leftover = 0, myideal = 0;
	char *cp, temp;

	myideal=strlen(myname);
	myideal=4*((8 + myideal +3)/4);

	more = nextDataBlock(&(pip->INODE), block_check++);
	//more is basically i_block[i]

	while(more)
	{	
		get_block(pip->dev, more, data_buf);
		printf("looking at data block %d\n", more);
		//step through this block to see if we can find an opening
		dp = (DIR*)data_buf;
		cp = data_buf;

		while(cp + dp->rec_len < data_buf + BLKSIZE)
		{
			temp = dp->name[dp->name_len];
			dp->name[dp->name_len] = 0;

			printf("rec len: %-4d -> ", dp->rec_len);
			temp = dp->name[dp->name_len];
			dp->name[dp->name_len] = 0;
			printf("%-15s -> ino(%d)\n", dp->name, dp->inode);

			if(strcmp(dp->name, myname) == 0)
			{
				dp->name[dp->name_len] = temp;
				printf("error this already exits, but was over written somehow.\n");
				return -1;
			}
			//printf("%s ", dp->name);
			dp->name[dp->name_len] = temp;
			//printf("ino(%d) rec_len(%d)\n", dp->inode, dp->rec_len);

			cp+=dp->rec_len;
			dp = (DIR*)cp;
		}
		printf("end rec_len(%d) ino(%d) \n", dp->rec_len, dp->inode);
		//now dp points to the last entry in data block
		ideal = ilen(dp);
		leftover = (dp->rec_len) - ideal;

		if(leftover >= myideal)
		{
			printf("found a spot!\n");
			//yeah we can fit in this data block!
			//increment dp pointer, set dp rec len to its ideal
			printf("Last entry only needs %d, but it has %d.\nIt can give up %d and I need %d.\n", ideal, dp->rec_len, leftover,myideal);

			dp->rec_len = ideal;
			
			cp+=ideal;
			dp = (DIR*)cp;

			//write this entry in
			dp->inode = myino;
			dp->rec_len = leftover;//ideal len of prev entry, entry gives up it's excess fat
			dp->name_len = strlen(myname);
	/*		printf("new file %s: ino %d, rec len: %d, namelen: %d", myname, dp->inode, dp->rec_len, dp->name_len);
			getchar();*/

			//write name to dp
			for(i = 0; i < dp->name_len; i++)
			{
				dp->name[i] = myname[i];
			}
			//cp = data_buf;
			//cp += 52;//wtf is this 52
			//dp = (DIR*)cp;

			//printf("new inode %d to %d!!!!!\n", dp->inode, more);
			//put this edited data_buf back to disk
			put_block(pip->dev, more, data_buf);
			return 1;
		}

		printf("can't fit in this this datablock %d\n", more);

		more = nextDataBlock(&(pip->INODE), block_check++);
	}

	block_check--;//this is the i_block that we need to balloc for
	
	if(block_check < 0)
	{
		printf("wtf happened in enter_name()\n");
		return -1;
	}
	//increment parent INO SIZE BY BLKSIZE!!!!!!!!(done in newDataBlock)
	//more must but 0 now
	//if(more == 0)

		i = newDataBlock(pip);//this will increment pip size and allocate

		printf("new data block: %d\n", i);
		getchar();
		if(i <= 0)
		{
			printf("no more blocks to allocate");
			return -1;
		}

		get_block(pip->dev, i, data_buf);

		dp = (DIR*)data_buf;

			//write this entry in
		dp->inode = myino;
		dp->rec_len = BLKSIZE;//ideal len of prev entry, entry gives up it's excess fat
		dp->name_len = strlen(myname);
		//dp->file_type = 0;
		//dp->name[0] = '.';
		//write name to dp
		for(i = 0; i < dp->name_len; i++)
		{
			dp->name[i] = myname[i];
		}
		//put this edited data_buf back to disk
		put_block(pip->dev, i, data_buf);
		return 1;		
}

//get ideal len of dir entry
int ilen(DIR *d)
{
	if(dp == 0)
	{
		return -1; //dp is null
	}

	int need_len = 4*((8 + d->name_len +3)/4);

	return need_len;
}

int tokenize(char *path, char *pieces[], int *npieces)
{
	char *p;
	char path_copy[MAX_PATH_LEN];

	*npieces = 0;
	//printf("entering tokenize function()\n");
	//to avoid "stack smashing; will occur if pLen > MAXpLen
	if(strlen((char*)path) > MAX_PATH_LEN)
	{
		printf("given path is too long, max 128\n");
		printf("exiting program ..\n");
		exit(1);
	}

	strcpy(path_copy, path);

	p = strtok(path_copy, "/");

	while(p != NULL)
	{
		pieces[(*npieces)] = (char*)malloc(sizeof(char)*(strlen(p) + 1));

		strcpy(pieces[(*npieces)++], p);
		p = strtok(NULL, "/");

		if((*npieces) - 1 == MAX_PATH_PIECES)
		{
			printf("too many folder redirections, %d max\n", MAX_PATH_PIECES - 1);
			printf("exiting program ..\n");
		
			exit(1);
		}
	}
	pieces[(*npieces)] = (char*)malloc(sizeof(char));
	pieces[(*npieces)] = 0;//null terminate array of strings
}

int tokCmd(char *line, char *myargv[], int *myargc)
{
	char *p;
	char line_copy[MAX_PATH_LEN];

	*myargc = 0;

	//to avoid "stack smashing; will occur if pLen > MAXpLen
	if(strlen(line) > MAX_PATH_LEN)
	{
		printf("given line too long, max 256 chars\n");
		//printf("exiting program ..\n");
		//exit(1);

		*myargc = 0;
		return 0;
	}

	strcpy(line_copy, line);

	p = strtok(line_copy, " ");


	while(p != NULL)
	{
		myargv[(*myargc)] = (char *)malloc(sizeof(char) * (strlen(p) + 1));
		strcpy(myargv[(*myargc)++], p);
					//printf("here->");getchar();
		p = strtok(NULL, " ");

		if((*myargc) - 1 == MAX_PATH_PIECES)
		{
			printf("args, %d max\n", MAX_PATH_PIECES - 1);
			printf("exiting program ..\n");
		
			*myargc = 0;
			return 0;
		}
	}

	myargv[(*myargc)] = (char *)malloc(sizeof(char));
	myargv[(*myargc)] = 0;//null terminate array of strings
}

//update this to work with current device
//currently only works for inodes in root device
int nextDataBlock(INODE *ip, int num)
{
	int i = 0, j = 0;		
	char level1[BLKSIZE];
	char level2[BLKSIZE];
	int *intp, *intp2; //int pointers for access data block data in buffer

	int count = 0, blocks = 0;

	if(ip->i_size % BLKSIZE)
	{
		blocks++;
	}
	blocks += ip->i_size / BLKSIZE;

	for(i = 0; i < 12 && i < blocks; i++, count++)
	{
		if(num == count)
		{
			return ip->i_block[i];
		}
		if(ip->i_block[i] == 0)
		{
			return 0;
		}
	}

		//block we're looking for doesn't exist
		//I should be 12 or less at this point
	if(i >= blocks)
	{
		//if here i is less than 12
		return 0;
	}

	//get blocks left
	blocks-= i;

	//i must be 12 here
	j = ip->i_block[12];

	//12 only has only level of indirect
	get_block(fd, j, level1);

	intp = (int *)level1;
	i = 0;

	//keep looking for the block you need
	while((int)intp < (int)level1 + (int)BLKSIZE && intp != 0 && *intp != 0)
	{
		if(num == count)
		{	
			return (*intp);
		}
		//can't go any farther so just return 0;
					if((*intp) == 0)
			{
				return 0;
			}

		intp++;
		i++;	
		count++;
	}

		//block we're looking for doesn't exist for this inode
		//
	if(i >= blocks)
	{
		return 0;
	}

	//get blocks left
	blocks-= i;

	j = ip->i_block[13];

	//block 13 has double indirect blocks
	get_block(fd, j, level1);
	intp = (int*)level1;
	i = 0;

	while((int)intp < (int)level1 + (int)BLKSIZE && intp != 0 && *intp != 0)
	{
		get_block(fd, *intp, level2);
		intp2 = (int *)level2;

		while((int)intp2 < (int)level2 + (int)BLKSIZE && intp2 != 0 && *intp2 != 0)
		{
			if(num == count)
			{
				return (*intp2);
			}

      				//can't go any further so just return now
			/*if((*intp2) == 0)
			{
					return 0;
				}	*/
			intp2++;//go to next in level two	
			i++;
			count++;
		}
		intp++;//go to next in level one
	}	
	
	blocks-= i;

	if(blocks > 0){return 0;} //we need to go to tripple indirect blocks

	//i dont think it can ever get to this line of code
	return 0;
}

int quit()
{
	int i = 0;

	for(i=0; i < NMINODES; i++)
	{
		if(minode[i].refCount > 1)
		{
			minode[i].refCount = 1;
			iput(&minode[i]);
		}

	}
	/*printf("dir: %s\n",dirname(pwd()));
	printf("base: %s\n", basename(pwd()));*/
	exit(1);
}

int pressEnterToContinue()
{
	char line[2];
	__fpurge(stdin);
	fgets(line, 2, stdin);
	__fpurge(stdin);
}

//will return the blk number of new allocated block
int newDataBlock(MINODE *pip)
{
	int i = 0, j = 0;		
		char level1[BLKSIZE];
		char level2[BLKSIZE];
		int *intp, *intp2; //int pointers for access data block data in buffer
		int k = 0;
		int count = 0, blocks = 0;
    bzero(data_buf, BLKSIZE);

		ip = &(pip->INODE);

		//keep current size until we 
		//check all blocks
		if(ip->i_size % BLKSIZE)
		{
			blocks++;
		}

		blocks += ip->i_size / BLKSIZE;

		for(i = 0; i < 12; i++, count++)
		{
			if(ip->i_block[i] == 0)
			{
				k = balloc(pip->dev);
				pip->dirty = 1;
				ip->i_block[i] = k;
        if(S_ISDIR(ip->i_mode))
				  ip->i_size+= BLKSIZE;
        //printf("%d\n", i);
				return k;
			}
		}

    if(ip->i_block[12] == 0)
    {
      //allocate block to store indirect blocks
      ip->i_block[12] = balloc(pip->dev);
      printf("indirect blocks begin\n");
      if(ip->i_block[12] < 0)
      {
        printf("no more datablock\n");
        return -1;
      }

      put_block(pip->dev, ip->i_block[12], data_buf);
    }

		//get blocks left
		blocks-= i;

		//i must be 12 here
		j = (ip->i_block[12]);

		//12 only has only level of indirect
		get_block(pip->dev, j, level1);

		intp = (int *)level1;
		i = 0;

		//keep looking for the block you need
		while((int)intp < (int)level1 + (int)BLKSIZE)
		{
			//won't ever have this, the i < blocks will cause break
			if((*intp) == 0)
			{	
        printf("new indirect block need\n");
				k = balloc(pip->dev);
				(*intp) = k;
				pip->dirty = 1;
        if(S_ISDIR(ip->i_mode))
				  ip->i_size+= BLKSIZE;
				put_block(pip->dev, j, level1);
				return k;
			}

			intp++;
			i++;	
			count++;
		}

		//not going this far  right now
		//printf("need double indirect, not doing that bull sh\n");
		//return -10;

		//get blocks left
		blocks-= i;

    if(ip->i_block[13] == 0)
    {
      //allocate block to store double indirect blocks
      ip->i_block[13] = balloc(pip->dev);
      printf("dub indirect will begin\n");
      if(ip->i_block[13] < 0)
      {
        printf("no more datablocka\n");
        return -1;
      }

      put_block(pip->dev, ip->i_block[13], data_buf);
    }

		j = ip->i_block[13];

		//block 13 has double indirect blocks
		get_block(pip->dev, j, level1);
		intp = (int*)level1;
		i = 0;

		while((int)intp < (int)level1 + (int)BLKSIZE)
		{
      if(*intp == 0)
      {
        //allocate block to store double indirect blocks

        *intp = balloc(pip->dev);
        printf("new  dub indirect block great\n");//getchar();
        if(*intp < 0)
        {
          printf("no more datablocka\n");
          return -1;
        }
        put_block(pip->dev, j, level1);
        put_block(pip->dev, *intp, data_buf);
      }

			get_block(pip->dev, *intp, level2);
			intp2 = (int *)level2;

			while((int)intp2 < (int)level2 + (int)BLKSIZE)
			{
				if((*intp2) == 0)
				{

				  k = balloc(pip->dev);
          if(k < 0)
          {
            printf("no more blocks\n");
            return -1;
          }

          *intp2 = k;
          put_block(pip->dev, *intp, level2); 

				  pip->dirty = 1;
          //printf("new dub indirect %d block great\n", k);getchar();
          if(S_ISDIR(ip->i_mode))
				    ip->i_size+= BLKSIZE;
				  return k;
				}

				//can't go any further so just return now
				/*if((*intp2) == 0)
				{
					return 0;
				}	*/
				intp2++;//go to next in level two	
				i++;
				count++;
			}
			intp++;//go to next in level one
		}	

		blocks-= i;

    if(blocks > 0)
    {
      printf("we need trip indirect which is probably wrong.\n");pressEnterToContinue();
      return -1;
    }

		printf("how tf did we get here?\n");
		return 0;
}

int nameExists(MINODE *parent, char *myname)
{
	char *cp, temp;
	int more, block_check = 0, i = 0;

	//if passed a null pointer
	if (parent == 0){printf("wtf null mide in fmyname()\n");return 0;}

	//int block_num, offset;
	//idont need to get inode from disk because it is already in memory
	//getInodeBlockNumberAndOffset(parent->ino, &block_num, &offset);
	
		//ip = (INODE *)inode_buf + 1;//get first inode which is root
	//load data block of root inode
	get_block(fd, (parent->INODE).i_block[block_check], data_buf);	
	
	dp = (DIR *)data_buf;
	cp = data_buf;//same as cp = data_buf

	more = nextDataBlock(&(parent->INODE), block_check++);
	
	//make sure we check all data blocks of dir, not just i_block[0]
	while(more)
	{
		more = nextDataBlock(&(parent->INODE), block_check++);

		while(cp < data_buf + BLKSIZE)
		{
			temp = dp->name[dp->name_len];
			dp->name[dp->name_len] = 0;

			if(strcmp(dp->name, myname) == 0)
			{
				dp->name[dp->name_len] = temp;
				return 1;
			}	

			dp->name[dp->name_len] = temp;
			//move cp and dp pointers
			cp+= dp->rec_len;
			dp = (DIR*)cp;
		}

		//if there's more get it, else we'll break on the next outer loop
		if(more)
		{
				get_block(parent->dev, more, data_buf);	
	
				dp = (DIR *)data_buf;
				cp = data_buf;//same as cp = data_buf
				//i could break here 
		}
	}

	return 0;
}

int creato(char *npath)
{
	char *ogpath = 0, parent[MAX_PATH_LEN], child[MAX_PATH_LEN];
	int check = 0;

	MINODE *pip;// = running->cwd;

	//printf("herex-->1");getchar();
	ogpath = (char *)malloc(sizeof(char) * (strlen(pwd() + 1) + 1));
	strcpy(ogpath, pwd());
	//printf("%s\n", ogpath);

	//printf("herex-->2");getchar();
	//printf("p:%s\n", npath);
	strcpy(child, (char*)basename(npath));
	//printf("child:%s\n", child);

	if(strlen(child) == 0)
	{
		printf("houston we have a problem, can't make dir here\n");
		return -1;
	}

	//printf("herex-->2");getchar();
	//printf("p:%s\n", npath);
	strcpy(parent, (char*)dirname(npath));
	//printf("prnt:%s\n", parent);


	//change dir to parent dir
	check = cd_wrap(parent);

	if(check < 0)
	{
		printf("can't make this file, bad path\n");
		return -1;
	}

	//
	pip = running->cwd;

	//try to make dir in parent directory, may fail if it exists or not enough data or inodes
	check = mycreat(pip, child);

	//goback to original working directory
	cd_wrap(ogpath);

  return check;
}

int mycreat(MINODE *pip, char *name)
{
	int i, ino;
	char *cp;
	MINODE *mip;

	if(pip == 0) 
	{
		printf("wtf parent mip is null\n");
		return -1;
	}

	ino = ialloc(pip->dev);
	//bno = balloc(pip->dev);

	if(ino < 0)
	{
		printf("no room for file: ino %d\n", ino);
		return -1;
	}

	//make sure name doesn't exist already
	if(nameExists(pip, name))
	{
		printf("file already exists\n");
		return -1;
	}

	//get this ino from memory
	//this ino will be blank
	//we need to set all of it's attributes
	mip = iget(pip->dev, ino);

	//now we have to make this inode a dir
	ip = &(mip->INODE);
	
	ip->i_mode = 0x81A4; //or 040755
	ip->i_uid = running->uid;
	ip->i_gid = running->gid;
	ip->i_size = 0;
	ip->i_links_count = 1;
	ip->i_atime = time(0L);
	ip->i_ctime = time(0L);
	ip->i_mtime = time(0L);
	ip->i_blocks = 2;//linux blocks count in 512-byte chunks
	//ip->i_block[0] = bno;

	for(i = 0; i <= 14; i++)
	{
		ip->i_block[i] = 0;
	}

	mip->dirty = 1;//mark bit as dirty so it gets written back to disk;

	iput(mip);

	printf("new ino will be %d\n", ino);
	return enter_name(pip, ino, name);
}

int creat_wrap(char *path)
{
	if(path == 0)
	{
		printf("give something to make\n");
		return -1;
	}

	while(strlen(path) > 1 && path[strlen(path) - 1] == '/')
	{
		//take off excess '/'
		printf("%s\n", path);
		path[strlen(path) - 1] = 0;
	}

		creato(path);
}

int rmdir_wrap(char *path)
{
	int myino = 0;

	if(path == 0)
	{
		printf("give something to remove\n");
		return -1;
	}

	while(strlen(path) > 1 && path[strlen(path) - 1] == '/')
	{
		//take off excess '/'
		printf("%s\n", path);
		path[strlen(path) - 1] = 0;
	}

	remove_dir(path);
}

int remove_dir(char *npath)
{
	char *ogpath = 0, parent[MAX_PATH_LEN], child[MAX_PATH_LEN], rmpath[MAX_PATH_LEN];
	int check = 0;
	int myino = 0;


	MINODE *pip, *mip;// = running->cwd;


	ogpath = (char *)malloc(sizeof(char) * (strlen(pwd() + 1) + 1));
	strcpy(ogpath, pwd());
	printf("%s\n", ogpath);


	//printf("path:%s\n", npath);
	strcpy(child, (char*)basename(npath));
	//printf("child:%s\n", child);

	if(strlen(child) == 0)
	{
		printf("houston we have a problem, can't rm dir\n");
		return -1;
	}

	//printf("path:%s\n", npath);
	strcpy(parent, (char*)dirname(npath));
	//printf("dirname:%s\n", parent);

	//change dir to parent dir
	check = cd_wrap(parent);

	if(check < 0)
	{
		printf("can't rm this no existent dir, bad path\n");
		return -1;
	}

	//
	pip = running->cwd;
	strcpy(rmpath, "");
	strcat(rmpath, pwd());
	strcat(rmpath, "/");
	strcat(rmpath, child);
	//printf("child's path:%s\n", rmpath);

	//see if this child has a path
	myino = getino(pip->dev, rmpath);

	//try to make dir in parent directory, may fail if it exists or not enough data or inodes
	myremovedir(pip, myino, child);

	//goback to original working directory
	cd_wrap(ogpath);
}

int myremovedir(MINODE *pip, int myino, char* myname)
{
	//in this function we will always be in the parent directory
	//pwd is where the thing is 
	int i;
	char *cp;
	MINODE *mip;

	if(myino == 0)
	{
		printf("this dir doesn't exist\n");
		return -1;
	}

	if(pip == 0) 
	{
		printf("wtf parent mip is null\n");
		return -1;
	}

	mip = iget(pip->dev, myino);

	if(!S_ISDIR(mip->INODE.i_mode))
	{
		iput(mip);
		printf("not a dir, can't remove\n");
		return -1;
	}

	//we know it exists at this points,but doesnt hurt to check again

	if(!emptyDir(pip, mip))
	{
		iput(mip);
		printf("not an empty dir!\n");
		return -1;

	if(mip->refCount > 1)
	{
		iput(mip);
		printf("BUSY refCount: %d. comeback later\n",mip->refCount);
		return -1;
	}
	//at this point file is an empty dir and not busyz

	//deallocate it's blocks
  for (i=0; i<12; i++){
    if (mip->INODE.i_block[i]==0)
        continue;
      bdealloc(mip->dev, mip->INODE.i_block[i]);
   }

  idealloc(mip->dev, mip->ino);
  iput(mip); //(which clears mip->refCount = 0);

	}

  rm_child(pip, myino, myname);
}

int rm_child(MINODE *pip, int myino, char *name)
{
	char *cp, *ncp, temp; 
	int more = 0, block_check = 0;
	int prec_len = 0, giveup = 0;
	int i = 0;
	DIR *ndp;
	more = nextDataBlock(&(pip->INODE), block_check++);
	//im assuming a dir doesn't have more than one block
	//while(more)
	//{
		get_block(pip->dev, more, data_buf);
		dp = (DIR *)data_buf;
		cp = data_buf;
		//printf("1\n");

		//getchar();
		while((int)cp + dp->rec_len <= (int)data_buf + BLKSIZE)
		{
			//printf("cp:%d data_buf +blksize:%d diff:%d\n", (int)cp, (int)data_buf + BLKSIZE, data_buf + BLKSIZE - cp);
			//null terminate name field of dp
			temp = dp->name[dp->name_len];
			dp->name[dp->name_len] = 0;
		
			if(strcmp(dp->name, name) == 0)
			{
				//printf("found ino(%d) to remove, it matchs dp->inode(%d)!!\n", myino, dp->inode);
				//we are point are the dir entry to remove
				//printf("cp:%d data_buf +blksize:%d diff:%d\n", (int)cp, (int)data_buf + BLKSIZE, data_buf + BLKSIZE - cp);//getchar();
				//printf("dp rec_len: %d\n", dp->rec_len);
				dp->name[dp->name_len] = temp;

				if((int)cp == (int)data_buf)
				{
					printf("it is first entry\n");
					giveup = dp->rec_len;
					printf("giveup %d rec_len\n", giveup);
					ndp = dp;
					ncp = (char*)dp;

					//these will be one entry ahead
					ncp+= dp->rec_len;
					ndp = (DIR*)ncp;

					//shift
					while((int)ncp < (int)data_buf + BLKSIZE)
					{
						//printf("3\n");
						if((int)ncp + ndp->rec_len >= (int)data_buf +BLKSIZE)
						{
							//give last extra len
							ndp->rec_len+=giveup;
						}
						//dp == nextdp
						memcpy(dp, ndp, ndp->rec_len);//memcpy(dp, ndp, sizeof(DIR));

						ncp+= ndp->rec_len;
						ndp = (DIR*)ncp;

						cp+= dp->rec_len;
						dp = (DIR*)cp;
					}
				}
				else if((int)(cp + (dp->rec_len)) == (int)(data_buf + BLKSIZE))
				{
					//printf("cp:%d dp->reclen%d: data_buf: %d db +blksize:%d\n", cp, dp->rec_len, data_buf, data_buf+BLKSIZE);
					printf("it is last entry\n");
					giveup = dp->rec_len;
					printf("giveup %d rec_len\n", giveup);

					//go back to previous
					cp-=prec_len;
					dp = (DIR*)cp;

					//give previous entry your love
					dp->rec_len+=giveup;
				}
				else if((int)(cp + (dp->rec_len)) < (int)(data_buf + BLKSIZE))
				{
					printf("\nin middle\n"); 
					giveup = dp->rec_len;
					printf("giveup %d rec_len\n", giveup);
					ndp = dp;
					ncp = (char*)dp;

					//these will be one entry ahead
					ncp+= dp->rec_len;
					ndp = (DIR*)ncp;
					//printf("check1\n");getchar();
					//printf("%d\n", ((int)data_buf + BLKSIZE) - (int)ncp);getchar();
					memcpy(dp, ncp, ((int)data_buf + BLKSIZE) - (int)ncp);
					//dp->rec_len += giveup;

					while((int)cp + dp->rec_len + giveup <= (int)data_buf + BLKSIZE)
					{
						if((int)cp + dp->rec_len + giveup == (int)data_buf + BLKSIZE)
						{
							dp->rec_len+=giveup;
						}
						cp += dp->rec_len;
						dp = (DIR*)cp;
					}
				}

				//write updated data block to disk
				printf("before put to block %d\n", more);
				put_block(pip->dev, more, data_buf);
				printf("after put to block (disk update complete)\n");
				return 0;
			}
			else{
				//printf("here");getchar();
				dp->name[dp->name_len] = temp;
			}

			
			prec_len = dp->rec_len;

			cp+=dp->rec_len;
			dp = (DIR*)cp;
			//printf("cp:%d data_buf +blksize:%d diff:%d\n", (int)cp, (int)data_buf + BLKSIZE, data_buf + BLKSIZE - cp);getchar();
		}

		//prec_len = 0;
		//more = nextDataBlock(&(pip->INODE), block_check++);
	//}

	printf("couldn't find");
	return -1;
}

int emptyDir(MINODE *pip, MINODE *mip)
{
	char *cp; 
	int more = 0, block_check = 0;

	if(!S_ISDIR(mip->INODE.i_mode))
	{
		printf("not a dir\n");
		return 0;
	}

	more = nextDataBlock(&(mip->INODE), block_check++);

	while(more)
	{
		get_block(mip->dev, more, data_buf);
		dp = (DIR *)data_buf;
		cp = data_buf;

		while((int)cp < (int)data_buf + BLKSIZE)
		{
			if(dp->inode != pip->ino && dp->inode != mip->ino)
			{
				printf("not empty\n");
				return 0;
			}


			cp+=dp->rec_len;
			dp = (DIR*)cp;
		}

		more = nextDataBlock(&(mip->INODE), block_check++);
	}
	//dir is empty
	printf("is empty\n");
	return 1;
}

int unlink_wrap(char *path)
{
	int myino = 0;

	if(path == 0)
	{
		printf("give something to remove\n");
		return -1;
	}

	while(strlen(path) > 1 && path[strlen(path) - 1] == '/')
	{
		//take off excess '/'
		printf("%s\n", path);
		path[strlen(path) - 1] = 0;
	}

	unlink(path);
}

int unlink(char *npath)
{
	char *ogpath = 0, parent[MAX_PATH_LEN], child[MAX_PATH_LEN], rmpath[MAX_PATH_LEN];
	int check = 0;
	int myino = 0;


	MINODE *pip, *mip;// = running->cwd;

	//printf("herex-->1");getchar();
	ogpath = (char *)malloc(sizeof(char) * (strlen(pwd() + 1) + 1));
	strcpy(ogpath, pwd());
	printf("%s\n", ogpath);

	//printf("herex-->2");getchar();
	//printf("p:%s\n", npath);
	strcpy(child, (char*)basename(npath));
	//printf("child:%s\n", child);

	if(strlen(child) == 0)
	{
		printf("houston we have a problem, can't unlink\n");
		return -1;
	}

	//printf("herex-->2");getchar();
	printf("p:%s\n", npath);
	strcpy(parent, (char*)dirname(npath));
	printf("prnt:%s\n", parent);



	//change dir to parent dir
	check = cd_wrap(parent);

	if(check < 0)
	{
		printf("can't unlink this non existent thing, bad path\n");
		return -1;
	}

	//
	pip = running->cwd;
	strcpy(rmpath, "");
	strcat(rmpath, pwd());
	strcat(rmpath, "/");
	strcat(rmpath, child);
	printf("child's path:%s\n", rmpath);

	//see if this child has a path
	myino = getino(pip->dev, rmpath);

	//try to make dir in parent directory, may fail if it exists or not enough data or inodes
	check = myunlink(pip, myino, child);

	//goback to original working directory
	cd_wrap(ogpath);

  return check;
}

//currently can't unlink a file who's inode is in oft
int myunlink(MINODE *pip, int myino, char *myname)
{
	//in this function we will always be in the parent directory
	//pwd is where the thing is 
	int i;
	char *cp;
	MINODE *mip;

	if(myino == 0)
	{
		printf("this file doesn't exist\n");
		return -1;
	}

	if(pip == 0) 
	{
		printf("wtf parent mip is null\n");
		return -1;
	}

	mip = iget(pip->dev, myino);
  //printf("refCount of %d is %d", mip->ino, mip->refCount);getchar();
	//if mip is null, can't link across 

	if(!(S_ISLNK(mip->INODE.i_mode) || S_ISREG(mip->INODE.i_mode)))
	{
		iput(mip);
		printf("only unlink links and reg files\n");
		return -1;
	}

  //currently I can't close any file that has the inode of an oft entry
	//we know it exists at this points,but doesnt hurt to check again
	if(mip->refCount > 1)
	{
		iput(mip);
		printf("BUSY ino(%d) refCount: %d. comeback later\n", mip->ino, mip->refCount);
		return -1;
	}

  for(i = 0; i < NFD; ++i)
  {
    if(running->fd[i])
    {
      if(running->fd[i]->inodeptr == mip)
      {
        iput(mip);
        printf("ino being used in OFT of running proc\n");
        return -1;
      }
    }
  }

	//at this point file is an empty dir and not busyz
	mip->INODE.i_links_count--;
	mip->dirty = 1;

	//deallocate it's blocks
	if(mip->INODE.i_links_count == 0)
  {
  		/*for (i=0; i<12; i++)
  		{
	    	if (mip->INODE.i_block[i]==0)
	    	{
	      		continue;
	    	}
      		bdealloc(mip->dev, mip->INODE.i_block[i]);
   		}
    	idealloc(mip->dev, mip->ino);*/
      truncateInode(mip);
 	}

  iput(mip); //(which clears mip->refCount = 0);

  return rm_child(pip, myino, myname);
}

int link_wrap(char *path1, char *path2, int symflag)
{
	int myino = 0;

	if(path1 == 0 || path2 == 0)
	{
		printf("give something to link\n");
		return -1;
	}

	while(strlen(path1) > 1 && path1[strlen(path1) - 1] == '/')
	{
		//take off excess '/'
		printf("%s\n", path1);
		path1[strlen(path1) - 1] = 0;
	}

	while(strlen(path2) > 1 && path2[strlen(path2) - 1] == '/')
	{
		//take off excess '/'
		printf("%s\n", path2);
		path2[strlen(path2) - 1] = 0;
	}

	link(path1, path2, symflag);
}

int link(char *path1, char *path2, int symflag)
{
	char *ogpath = 0, parent1[MAX_PATH_LEN], parent2[MAX_PATH_LEN], child1[MAX_PATH_LEN], child2[MAX_PATH_LEN], 
	lnpath1[MAX_PATH_LEN], lnpath2[MAX_PATH_LEN];
	int check = 0;
	int myino = 0;


	MINODE *pip, *mip;// = running->cwd;

	//hardlink

		//malloc room to fit current working dir before we cd behind the scenes
		ogpath = (char *)malloc(sizeof(char) * (strlen(pwd() + 1) + 1));
		strcpy(ogpath, pwd());
		printf("%s\n", ogpath);

		//printf("herex-->2");getchar();
		//printf("p:%s\n", npath);
		strcpy(child1, (char*)basename(path1));
		strcpy(child2, (char*)basename(path2));
		//printf("child:%s\n", child);

		if(strlen(child1) == 0 || strlen(child2) == 0)
		{
			printf("houston we have a problem, can't unlink\n");
			return -1;
		}

		printf("p1:%s\n", path1);
		strcpy(parent1, (char*)dirname(path1));
		printf("prnt1:%s\n", parent1);

		printf("p2:%s\n", path2);
		strcpy(parent2, (char*)dirname(path2));
		printf("prnt2:%s\n", parent2);

		cd_wrap(ogpath);
		//change dir to parent dir
		check = cd_wrap(parent1);

		if(check < 0)
		{
			printf("can't link this non existent thing, bad path\n");
			return -1;
		}

		pip = running->cwd;
		strcpy(lnpath1, "");
		strcat(lnpath1, pwd());
		strcat(lnpath1, "/");
		strcat(lnpath1, child1);
		printf("child1's path:%s\n", lnpath1);

		//get ino of the thing we are linking to
		myino = getino(pip->dev, lnpath1);
		cd_wrap(ogpath);
		//change dir to parent dir
		check = cd_wrap(parent2);

		if(check < 0)
		{
			printf("can't link this non existent thing, bad path\n");
			//cd_wrap(ogpath);
			return -1;
		}

		//pip should still being pointing to
		pip = running->cwd;

		if(symflag == 0)
		{
			mylink(pip, myino, child2);
		}
		else 
		{
			mysymlink(pip, lnpath1, child2);
		}

		//goback to original working directory
		cd_wrap(ogpath);
		return;


	//sym link
}

int mylink(MINODE *pip, int myino, char *myname)
{
	//in this function we will always be in the parent directory
	//pwd is where the thing is 
	int i;
	char *cp;
	MINODE *mip;

	if(myino == 0)
	{
		printf("this file doesn't exist\n");
		return -1;
	}

	if(pip == 0) 
	{
		printf("wtf parent mip is null\n");
		return -1;
	}

	mip = iget(pip->dev, myino);

	//if mip is null, can't link across 

	if(!(S_ISLNK(mip->INODE.i_mode) || S_ISREG(mip->INODE.i_mode)))
	{
		iput(mip);
		printf("only link links and reg files\n");
		return -1;
	}
	mip->INODE.i_links_count++;
	mip->dirty = 1;

  iput(mip); //(which clears mip->refCount = 0);

  enter_name(pip, myino, myname);
}

int mysymlink(MINODE *pip, char *oldname, char *name)
{
	int i, ino, oldlen = 0;
	char *cp, onamecpy[MAX_PATH_LEN];
	MINODE *mip;
	
	if(pip == 0) 
	{
		printf("wtf parent mip is null\n");
		return -1;
	}

	bzero(onamecpy, MAX_PATH_LEN);
	strcpy(onamecpy, oldname);

	ino = ialloc(pip->dev);
	//bno = balloc(pip->dev);

	if(ino < 0)
	{
		printf("no room for file: ino %d\n", ino);
		return -1;
	}

	//make sure name doesn't exist already
	if(nameExists(pip, name))
	{
		printf("file already exists\n");
		return -1;
	}

	//get this ino from memory
	//this ino will be blank
	//we need to set all of it's attributes
	mip = iget(pip->dev, ino);

	//now we have to make this inode a dir
	ip = &(mip->INODE);
	
	ip->i_mode = 0xA1A4; //make link file
	ip->i_uid = running->uid;
	ip->i_gid = running->gid;
	ip->i_size = strlen(oldname);
	ip->i_links_count = 1;
	ip->i_atime = time(0L);
	ip->i_ctime = time(0L);
	ip->i_mtime = time(0L);
	ip->i_blocks = 2;//linux blocks count in 512-byte chunks
	//ip->i_block[0] = bno;

	//TRYING TO write old path to iblocks
	for(i = 0; i <= 14; i++)
	{
		ip->i_block[i] = 0;
	}

	oldlen = strlen(onamecpy);

	memcpy(&(ip->i_block), &onamecpy, oldlen);
	//printf("%s\n", ip->i_blocks);
	//getchar();
	mip->dirty = 1;//mark bit as dirty so it gets written back to disk;

	iput(mip);

	printf("new ino for symlink will be %d\n", ino);
	enter_name(pip, ino, name);
}

int open_file_wrap(char *path, char *mode)
{
	int fmode = 0;
  //printf("herex-->1 %s", path);getchar();
	if(path == 0 || mode == 0)
	{
		printf("give me something to open\n");
		return -1;
	}

	//remove excess / at the end of string. i found this to cause problems so i just always remove the trailing '/'s
	while(strlen(path) > 1 && path[strlen(path) - 1] == '/')
	{
		path[strlen(path) - 1] = 0;
	}

	//convert mode string into mode int
	if(strcmp(mode, "0") == 0)
	{
		fmode = 0;//read
	}
	else if (strcmp(mode, "1") == 0)
	{
		fmode = 1;//write
	}
	else if (strcmp(mode, "2") == 0)
	{
		fmode = 2;//readwrite
	}
	else if (strcmp(mode, "3") == 0)
	{
		fmode = 3;//append
	}
	else
	{
		fmode = -1;
	}

	if(-1 == fmode)
	{
		printf("invalid file mode\n");
		return -1;
	}


	return open_file(path, fmode);
}

int open_file(char *path, int mode)
{
	char *ogpath = 0, parent[MAX_PATH_LEN], child[MAX_PATH_LEN], openpath[MAX_PATH_LEN];
	int check = 0;
	int myino = 0;
  int newfd = 0;


	MINODE *pip, *mip;// = running->cwd;


	ogpath = (char *)malloc(sizeof(char) * (strlen(pwd() + 1) + 1));
	strcpy(ogpath, pwd());
	printf("%s\n", ogpath);

	//printf("herex-->2");getchar();
	//printf("p:%s\n", npath);
	strcpy(child, (char*)basename(path));
	//printf("child:%s\n", child);

	if(strlen(child) == 0)
	{
		printf("houston we have a problem, can't open_file\n");
		return -1;
	}

	//printf("p:%s\n", path);
	strcpy(parent, (char*)dirname(path));
	//printf("prnt:%s\n", parent);

	//change dir to parent dir
	check = cd_wrap(parent);

	if(check < 0)
	{
		printf("can't open this, bad path\n");
		return -1;
	}

	//
	pip = running->cwd;
	strcpy(openpath, "");
	strcat(openpath, pwd());
	strcat(openpath, "/");
	strcat(openpath, child);
	printf("child's path:%s\n", openpath);

	//see if this child has a path
	myino = getino(pip->dev, openpath);

	if(myino <= 0)
	{
		printf("file doesn't exist, bad path\n");
		return -1;
	}

	//will have to change this pip->dev to something else
	mip = iget(pip->dev, myino);

	//try to make dir in parent directory, may fail if it exists or not enough data or inodes
	newfd = myopen_file(mip, mode);

  //don't put because oft is going to keep refrencing it
  //will i put if function ffails in myopen file
	//iput(mip);
	//goback to original working directory
	cd_wrap(ogpath);

  return newfd;
}

int myopen_file(MINODE *mip, int mode)
{   
	int i = 0;
  OFT *opfita;

  //printf("refCount of %d is %d", mip->ino, mip->refCount);getchar();

	if(!S_ISREG(mip->INODE.i_mode))
	{
		printf("only open reg files\n");
    iput(mip);
		return -1;
	}

		//if it's not read we have to make sure it's not already open
	if(mode != 0)
	{
		while(i < NFD)
		{	
			if(running->fd[i])
			{
				//check is file is already opoen
				if(running->fd[i]->inodeptr == mip)
				{
					printf("can't open file for 1 2 or 3 if already open\n");
          iput(mip);
					return -1;
				}
			}
			//else th fd is open
			i++;
		}
	}
  else
  {
    for (i = 0; i < NFD; ++i)
    {
      if(running->fd[i])
      {
        if(running->fd[i]->inodeptr == mip && running->fd[i] != 0)
        {
          printf("file already open for something else, can't open for read\n");
          iput(mip);
          return -1;
        }
      }
    }
  }

  //currently I can open a descriptor to read a file
  //that is being written to

	i = 0;
	while(running->fd[i] && i < NFD)
	{
		i++;
	}
	//keep i as is for reference to the oft index in current proc's fd array
	if(i == NFD)
	{
		printf("no free fd's on running process\n");
    iput(mip);
		return -1;
	}

  opfita = (OFT *)malloc(sizeof(OFT));//might not have to do this
  opfita = getOFT();

	/*running->fd[i]->mode = mode;
	running->fd[i]->refCount = 1;
	running->fd[i]->inodeptr = mip;*/

  opfita->mode = mode;
  opfita->refCount = 1;
  opfita->inodeptr = mip;

	switch(mode)
	{
		case 0: opfita->offset = 0;//read mode
			break;
		case 1: truncateInode(mip); //WR truncate file to 0 size
			opfita->offset = 0;
			break;
		case 2: opfita->offset = 0; //RW does not truncate file
			break;
		case 3: opfita->offset = mip->INODE.i_size; //APPEND MODE
			break;
		default: printf("invalid mode\n");
			return -1;
      break;
	}

  running->fd[i] = opfita;

  //i don't know which time field i need to update?
  running->fd[i]->inodeptr->INODE.i_atime = time(0);
  running->fd[i]->inodeptr->INODE.i_ctime = time(0);
  running->fd[i]->inodeptr->INODE.i_mtime = time(0);

  printf("fd %d opened\n", i);
    //printf("refCount of %d is %d", mip->ino, mip->refCount);getchar();
  //mylseek(i, 5);
  return i;
}

int truncateInode(MINODE* mip)
{
	int i = 0, j = 0;    
  char level1[BLKSIZE];
  char level2[BLKSIZE];
  int *intp, *intp2; //int pointers for access data block data in buffer
  int k = 0;
  int count = 0, blocks = 0;
  bzero(data_buf, BLKSIZE);

    ip = &(mip->INODE);

    //keep current size until we 
    //check all blocks
    if(ip->i_size % BLKSIZE)
    {
      blocks++;
    }

    blocks += ip->i_size / BLKSIZE;

    for(i = 0; i < 12; i++, count++)
    {
      if(ip->i_block[i] == 0)
      {
       return 0;
      }
      
      bdealloc(mip->dev, ip->i_block[i]);
      put_block(mip->dev, ip->i_block[i], data_buf);
      mip->dirty = 1;
      ip->i_block[i] = 0;
    }

    if(ip->i_block[12] == 0)
    {
      return 0;
    }

    //get blocks left
    blocks-= i;

    //i must be 12 here
    j = (ip->i_block[12]);

    //12 only has only level of indirect
    get_block(mip->dev, j, level1);
    bdealloc(mip->dev, j);

    intp = (int *)level1;
    i = 0;

    //keep looking for the block you need
    while((int)intp < (int)level1 + (int)BLKSIZE)
    {
      //won't ever have this, the i < blocks will cause break
      if((*intp) == 0)
      { 
        //clear iblock 12 
        put_block(mip->dev, j, data_buf);
        return 0;
      }

      bdealloc(mip->dev, *intp);
      put_block(mip->dev, *intp, data_buf);
      (*intp) = 0;
      

      intp++;
      i++;  
      count++;
    }

    //clear level one of 12
    put_block(mip->dev, j, data_buf);
    //not going this far  right now
    //printf("need double indirect, not doing that bull sh\n");
    //return -10;

    //get blocks left
    blocks-= i;



    if(ip->i_block[13] == 0)
    {
      return 0;
      //allocate block to store double indirect blocks
  /*    ip->i_block[13] = balloc(pip->dev);
      printf("dub indirect will begin\n");
      if(ip->i_block[13] < 0)
      {
        printf("no more datablocka\n");
        return -1;
      }

      put_block(pip->dev, ip->i_block[13], data_buf);*/
    }

    j = ip->i_block[13];
    ip->i_block[13] = 0;

    //block 13 has double indirect blocks
    get_block(mip->dev, j, level1);
    bdealloc(mip->dev, j);
    put_block(mip->dev, j, data_buf); 
    //got iblock13 in memory, now clear it on disk

    intp = (int*)level1;
    i = 0;

    printf("dellocing double indirect\n");pressEnterToContinue();
    while((int)intp < (int)level1 + (int)BLKSIZE)
    {
      if(*intp == 0)
      {
        //clear block j on disk which help level 1 ints
        //put_block(mip->dev, j, data_buf); 
        return 0;
        //allocate block to store double indirect blocks
  /*        return 0;
        *intp = balloc(pip->dev);
        printf("new  dub indirect block great\n");getchar();
        if(*intp < 0)
        {
          printf("no more datablocka\n");
          return -1;
        }
        put_block(pip->dev, j, level1);
        put_block(pip->dev, *intp, data_buf);*/
      }
      else
      {
        //deallocate a block from 1st level
        //get clock
        get_block(mip->dev, *intp, level2);
        intp2 = (int *)level2;

        //then deallocate it on disk because we have it in memory
        bdealloc(mip->dev, *intp);
        //clear this block
        put_block(mip->dev, *intp, data_buf);
        *intp = 0; 
      }



      while((int)intp2 < (int)level2 + (int)BLKSIZE)
      {
        if((*intp2) == 0)
        {
         
          return 0;
       /*   k = balloc(pip->dev);
          if(k < 0)
          {
            printf("no more blocks\n");
            return -1;
          }

          *intp2 = k;
          put_block(pip->dev, *intp, level2); 

          pip->dirty = 1;
          //printf("new dub indirect %d block great\n", k);getchar();
          if(S_ISDIR(ip->i_mode))
            ip->i_size+= BLKSIZE;
          return k;*/
        }
        else
        {
          bdealloc(mip->dev, *intp2);
          put_block(mip->dev, *intp2, data_buf);
          *intp2 = 0;
          put_block(mip->dev, *intp, level2); 
        }

        //can't go any further so just return now
        /*if((*intp2) == 0)
        {
          return 0;
        } */
        intp2++;//go to next in level two 
        i++;
        count++;
      }
      intp++;//go to next in level one
    } 

    blocks-= i;

    printf("this file is huge wtd\n");pressEnterToContinue();
    ip->i_block[14] = 0;
    return 0;

    if(blocks > 0)
    {

      printf("we need trip indirect which is probably wrong.\n");pressEnterToContinue();
      return -1;
    }

    printf("how tf did we get here?\n");
    return 0;
}

OFT* getOFT()
{
  int i = 0;

  for(i = 0; i < NOFT;i++)
  {

    if(oft[i].refCount == 0)
    {
      return &(oft[i]);
    }

  }

  return 0;
}

int close_file_wrap(char *fde)
{
  int fdesc = 0;

  if(fde == 0)
  {
    printf("give me something to close\n");
    return -1;
  }

  //convert mode string into mode int
  if(strcmp(fde, "0") == 0)
  {
    fdesc = 0;//read
  }
  else if (strcmp(fde, "1") == 0)
  {
    fdesc = 1;//write
  }
  else if (strcmp(fde, "2") == 0)
  {
    fdesc = 2;//readwrite
  }
  else if (strcmp(fde, "3") == 0)
  {
    fdesc = 3;//append
  }
   else if (strcmp(fde, "4") == 0)
  {
    fdesc = 4;//append
  }
   else if (strcmp(fde, "5") == 0)
  {
    fdesc = 5;//append
  }
   else if (strcmp(fde, "6") == 0)
  {
    fdesc = 6;//append
  }
   else if (strcmp(fde, "7") == 0)
  {
    fdesc = 7;//append
  }
   else if (strcmp(fde, "8") == 0)
  {
    fdesc = 8;//append
  }
   else if (strcmp(fde, "9") == 0)
  {
    fdesc = 9;//append
  }
  else
  {
    fdesc = -1;
  }

  if(-1 == fdesc)
  {
    printf("out of range or unacceptable\n");
    return -1;
  }

  close_file(fdesc);
}

int close_file(int fdesc)
{
  myclose_file(fdesc);
}

int myclose_file(int fdesc)
{
  MINODE *mip;

  if(running->fd[fdesc] && running->fd[fdesc]->refCount > 0)
  {
    running->fd[fdesc]->refCount--;

    if(running->fd[fdesc]->refCount > 0)
    {
      return 0;
    }

    mip = running->fd[fdesc]->inodeptr;
    running->fd[fdesc]->refCount = 0;//redundant
    iput(mip);
    running->fd[fdesc]= 0;  

    return 0;
  }
  else 
  {
    printf("this fd is not open\n");
    return -1;
  }
}

int seek_fd_wrap(char *fde, char *stringseek)
{
  int fdesc = 0, seek = 0;

  if(fde == 0 || stringseek== 0)
  {
    printf("need fd and seek position\n");
    return -1;
  }

  fdesc = atoi(fde);

  if(fdesc == 0 && strcmp(fde, "0") != 0)
  {
    printf("invalid fd\n");
    return -1;
  }

  seek = atoi(stringseek);

  if(seek == 0 && strcmp(stringseek, "0") != 0)
  {
    printf("invalid seek position\n");
    return -1;
  }

  mylseek(fdesc, seek);
}


int mylseek(int fdesc, int position)
{
  MINODE *mip;
  int ogposition;

  if(fdesc < 0 || fdesc > 9)
  {
    printf("this fd is out of range\n");
    return -1;
  }

  //if it's null or ref count == 0, this fd is not being used return
  if(!(running->fd[fdesc]) || running->fd[fdesc]->refCount == 0)
  {
    printf("this fd isn't open\n");
    return -1;
  }

  mip = running->fd[fdesc]->inodeptr;
  ogposition = running->fd[fdesc]->offset;

  if(position <= mip->INODE.i_size && position >= 0)
  {
    running->fd[fdesc]->offset = position;
  }
  else if(position > mip->INODE.i_size)
  {
    running->fd[fdesc]->offset = mip->INODE.i_size;
    printf("seeked beyond file (we go to end)\n");
  }
  else
  {
    printf("can't seek negative position.\n");
    return -1;
  }

  return ogposition;
}

int pfd()
{
  int i = 0;
  printf("fd\tmode\toffset\tINODE\n");
  for (i = 0; i < NFD; i++)
  {
    if(running->fd[i])
    {
      printf("%d\t%d\t%d\t[%d, %d]\n", i, running->fd[i]->mode, running->fd[i]->offset, running->fd[i]->inodeptr->dev, running->fd[i]->inodeptr->ino);
    }
  }
}

int dup_fd_wrap(char *fde)
{
  int fdesc = 0;

  if(fde == 0)
  {
    printf("give me something to dup\n");
    return -1;
  }

  //convert mode string into mode int
  if(strcmp(fde, "0") == 0)
  {
    fdesc = 0;//read
  }
  else if (strcmp(fde, "1") == 0)
  {
    fdesc = 1;//write
  }
  else if (strcmp(fde, "2") == 0)
  {
    fdesc = 2;//readwrite
  }
  else if (strcmp(fde, "3") == 0)
  {
    fdesc = 3;//append
  }
   else if (strcmp(fde, "4") == 0)
  {
    fdesc = 4;//append
  }
   else if (strcmp(fde, "5") == 0)
  {
    fdesc = 5;//append
  }
   else if (strcmp(fde, "6") == 0)
  {
    fdesc = 6;//append
  }
   else if (strcmp(fde, "7") == 0)
  {
    fdesc = 7;//append
  }
   else if (strcmp(fde, "8") == 0)
  {
    fdesc = 8;//append
  }
   else if (strcmp(fde, "9") == 0)
  {
    fdesc = 9;//append
  }
  else
  {
    fdesc = -1;
  }

  if(-1 == fdesc)
  {
    printf("out of range or unacceptable fd\n");
    return -1;
  }

  dup_fd(fdesc);
}

int dup_fd(int fdesc)
{
  mydup_fd(fdesc);
}

int mydup_fd(int fdesc)
{
  int i = 0;

  if(running->fd[fdesc] && running->fd[fdesc]->refCount > 0)
  {
    if(running->fd[fdesc]->mode != 0)
    {
      printf("can only dup read descriptors\n");
      return -1;
    }

    for (i = 0; i < NFD; ++i)
    {
      //found unused file descriptor
      if(running->fd[i] == 0)
      {
        break;
      }
    }

    if(i >= NFD)
    {
      printf("no open fd's\n");
      return -1;
    }

    running->fd[i] = (OFT*)malloc(sizeof(OFT));
    memcpy(running->fd[i], running->fd[fdesc], sizeof(OFT));
    running->fd[i]->inodeptr = iget(running->fd[fdesc]->inodeptr->dev, running->fd[fdesc]->inodeptr->ino);
  }
  else 
  {
    printf("this fd is not open\n");
    return -1;
  }
}

//-------
int dup2_fd_wrap(char *fde, char *gde)
{
  int fdesc = 0, gdesc = 0;

  if(fde == 0 || gde == 0)
  {
    printf("missing something for dup2\n");
    return -1;
  }

  //convert mode string into mode int
  if(strcmp(fde, "0") == 0)
  {
    fdesc = 0;//read
  }
  else if (strcmp(fde, "1") == 0)
  {
    fdesc = 1;//write
  }
  else if (strcmp(fde, "2") == 0)
  {
    fdesc = 2;//readwrite
  }
  else if (strcmp(fde, "3") == 0)
  {
    fdesc = 3;//append
  }
   else if (strcmp(fde, "4") == 0)
  {
    fdesc = 4;//append
  }
   else if (strcmp(fde, "5") == 0)
  {
    fdesc = 5;//append
  }
   else if (strcmp(fde, "6") == 0)
  {
    fdesc = 6;//append
  }
   else if (strcmp(fde, "7") == 0)
  {
    fdesc = 7;//append
  }
   else if (strcmp(fde, "8") == 0)
  {
    fdesc = 8;//append
  }
   else if (strcmp(fde, "9") == 0)
  {
    fdesc = 9;//append
  }
  else
  {
    fdesc = -1;
  }

   if(strcmp(gde, "0") == 0)
  {
    gdesc = 0;//read
  }
  else if (strcmp(gde, "1") == 0)
  {
    gdesc = 1;//write
  }
  else if (strcmp(gde, "2") == 0)
  {
    gdesc = 2;//readwrite
  }
  else if (strcmp(gde, "3") == 0)
  {
    gdesc = 3;//append
  }
   else if (strcmp(gde, "4") == 0)
  {
    gdesc = 4;//append
  }
   else if (strcmp(gde, "5") == 0)
  {
    gdesc = 5;//append
  }
   else if (strcmp(gde, "6") == 0)
  {
    gdesc = 6;//append
  }
   else if (strcmp(gde, "7") == 0)
  {
    gdesc = 7;//append
  }
   else if (strcmp(gde, "8") == 0)
  {
    gdesc = 8;//append
  }
   else if (strcmp(gde, "9") == 0)
  {
    gdesc = 9;//append
  }
  else
  {
    gdesc = -1;
  }

  if(-1 == fdesc || -1 == gdesc)
  {
    printf("fd or gd out of range or unacceptable fd\n");
    return -1;
  }

  dup2_fd(fdesc, gdesc);
}

int dup2_fd(int fdesc, int gdesc)
{
  mydup2_fd(fdesc, gdesc);
}

int mydup2_fd(int fdesc, int gdesc)
{
  int i = 0;

  if(running->fd[fdesc] && running->fd[fdesc]->refCount > 0)
  {
    if(running->fd[fdesc]->mode != 0)
    {
      printf("can only dup read descriptors\n");
      return -1;
    }

    myclose_file(gdesc);

    running->fd[gdesc] = (OFT*)malloc(sizeof(OFT));
    memcpy(running->fd[gdesc], running->fd[fdesc], sizeof(OFT));
    running->fd[gdesc]->inodeptr = iget(running->fd[fdesc]->inodeptr->dev, running->fd[fdesc]->inodeptr->ino);
  }
  else 
  {
    printf("this fd is not open\n");
    return -1;
  }
}

int read_file_wrap(char *fde, char *stringbytes)
{
  int fdesc = 0, bytes = 0;

  if(fde == 0 || stringbytes == 0)
  {
    printf("need fd and #bytes to read\n");
    return -1;
  }

  fdesc = atoi(fde);

  if(fdesc == 0 && strcmp(fde, "0") != 0)
  {
    printf("invalid fd\n");
    return -1;
  }

  bytes = atoi(stringbytes);

  if(bytes == 0 && strcmp(stringbytes, "0") == 0)
  {
    printf("why would I read 0 bytes\n");
    return -1;
  }
  else if(bytes == 0)
  {
    printf("invalid in for bytes to read\n");
    return -1;
  }

  read_file(fdesc, bytes);
}

int read_file(int fdesc, int bytes)
{
  myread_file(fdesc, bytes);
}

int myread_file(int fdesc, int bytes)
{
  MINODE *mip;
  int avail = 0, remain = 0;
  int offset = 0, start = 0;
  int lbk = 0;
  char *cp;
  int more = 0, block_check = 0, i =0;
  int desired = 0;
  char buf[BLKSIZE+1];

  bzero(data_buf, BLKSIZE);
  if(fdesc > NFD || fdesc < 0)
  {
    printf("fd to read is out of range\n");
    return -1;
  }

  if(running->fd[fdesc] == 0 || running->fd[fdesc]->refCount == 0)
  {
    printf("%d is not open\n", fdesc);
    return -1;
  }

  if(running->fd[fdesc]->mode != 0 && running->fd[fdesc]->mode != 2)
  {
    printf("file descriptor %d is not open for read (0 or 2)\n", fdesc);
    return -1;
  }

  //at this point our fd is open and open for read more
  mip = running->fd[fdesc]->inodeptr;
  offset = running->fd[fdesc]->offset;

  if(offset == mip->INODE.i_size)
  {
    printf("no more to read\n");
    return 0;
  }
  if(offset > mip->INODE.i_size)
  {
    printf("no more to read. how tf did this happen?\n");
    return 0;
  }

  avail = mip->INODE.i_size - offset;
  lbk = offset / BLKSIZE;
  start = offset % BLKSIZE;
  remain = bytes;
  desired = bytes;

  if(desired > avail)
  {
    printf("you want to read %d, but only %d available.", desired, avail);pressEnterToContinue();
    desired = avail;
  }

  remain = desired;

  bzero(data_buf, BLKSIZE);
  bzero(buf, BLKSIZE + 1);
  more = nextDataBlock(&(mip->INODE), lbk++);
  get_block(mip->dev, more, data_buf);
  cp = data_buf;
  cp+= start;
  //printf("%d\n", start);getchar();

  //getting first logical block
  if(remain >= BLKSIZE - start)
  {
      memcpy(buf, cp, (data_buf + BLKSIZE) - cp );
      (running->fd[fdesc]->offset) += (BLKSIZE - start);
      remain-=(BLKSIZE - start);   
  }
  else
  {
    memcpy(buf, cp, remain);
    running->fd[fdesc]->offset += remain;
    remain = 0;  
    //printf("%s", buf);
    printf("\nmy read: read %d char from fd %d\n", desired, fdesc);
    return desired;
  }

  //printf("%s", buf);

  more = nextDataBlock(&(mip->INODE), lbk++);
  //getchar();

  while(more && remain)
  {
    bzero(buf, BLKSIZE);

    //printf("-->more: %d<--", more);getchar();
    get_block(mip->dev, more, data_buf);

    if(remain >= BLKSIZE)
    {
      memcpy(buf, data_buf, BLKSIZE);
      running->fd[fdesc]->offset += BLKSIZE;
      remain-=BLKSIZE;
    }
    else
    {
      memcpy(buf, data_buf, remain);
      running->fd[fdesc]->offset += remain;
      remain = 0;
    }

    more = nextDataBlock(&(mip->INODE), lbk++);
    //printf("%s", buf);

    if(!more && remain)
    {
      printf("wtf no more blocks to read but we need more\n");
    }

  }


  //buf[i]
  //printf("%s\n", buf);
  printf("\nmy read: read %d char from fd %d\n", desired, fdesc);
  return desired;
}

int cat_file_wrap(char *path)
{
  int fdesc = 0;

  if(path == 0)
  {
    printf("give me something to cat\n");
    return -1;
  }

  while(strlen(path) > 1 && path[strlen(path) - 1] == '/')
  {
    path[strlen(path) - 1] = 0;
  }

  fdesc = open_file(path, 0);

  if(fdesc < 0)
  {
    printf("can't open file to cat\n");
    return -1;
  }

  cat_file(fdesc);
  close_file(fdesc);
}

int cat_file(int fdesc)
{
  mycat_file(fdesc);
}

int mycat_file(int fdesc)
{
  MINODE *mip;
  int avail = 0, remain = 0;
  int offset = 0, start = 0;
  int lbk = 0;
  char *cp;
  int more = 0, block_check = 0, i =0;
  int desired = 0;
  char buf[BLKSIZE+1];

  bzero(data_buf, BLKSIZE);
  if(fdesc > NFD || fdesc < 0)
  {
    printf("fd to read is out of range\n");
    return -1;
  }

  if(running->fd[fdesc] == 0 || running->fd[fdesc]->refCount == 0)
  {
    printf("%d is not open\n", fdesc);
    return -1;
  }

  if(running->fd[fdesc]->mode != 0)
  {
    printf("file descriptor %d is not open for read (0)\n", fdesc);
  }

  //at this point our fd is open and open for read more
  mip = running->fd[fdesc]->inodeptr;
  offset = 0;//running->fd[fdesc]->offset;

  if(offset == mip->INODE.i_size)
  {
    //printf("nothing to cat\n");
    return 0;
  }
  if(offset > mip->INODE.i_size)
  {
    printf("no more to read. how tf did this happen?\n");
    return 0;
  }

  avail = mip->INODE.i_size;
  lbk = offset / BLKSIZE;
  start = offset % BLKSIZE;
  remain = avail;
  desired = avail;

  if(desired > avail)
  {
    printf("you want to read %d, but only %d available.", desired, avail);pressEnterToContinue();
    desired = avail;
  }

  remain = desired;

  bzero(data_buf, BLKSIZE);
  bzero(buf, BLKSIZE + 1);
  more = nextDataBlock(&(mip->INODE), lbk++);
  get_block(mip->dev, more, data_buf);
  cp = data_buf;
  cp+= start;

  //getting first logical block
  if(remain >= BLKSIZE - start)
  {
      memcpy(buf, cp, (data_buf + BLKSIZE) - cp );
      (running->fd[fdesc]->offset) += (BLKSIZE - start);
      remain-=(BLKSIZE - start);   
  }
  else
  {
    memcpy(buf, cp, remain);
    running->fd[fdesc]->offset += remain;
    remain = 0;  
    printf("%s", buf);
    //printf("\nmy read: read %d char from fd %d\n", desired, fdesc);
    return desired;
  }

  printf("%s", buf);

  more = nextDataBlock(&(mip->INODE), lbk++);
  //getchar();

  while(more && remain)
  {
    bzero(buf, BLKSIZE);

    //printf("-->more: %d<--", more);getchar();
    get_block(mip->dev, more, data_buf);

    if(remain >= BLKSIZE)
    {
      memcpy(buf, data_buf, BLKSIZE);
      running->fd[fdesc]->offset += BLKSIZE;
      remain-=BLKSIZE;
    }
    else
    {
      memcpy(buf, data_buf, remain);
      running->fd[fdesc]->offset += remain;
      remain = 0;
    }

    more = nextDataBlock(&(mip->INODE), lbk++);
    printf("%s", buf);

    if(!more && remain)
    {
      printf("wtf no more blocks to read but we need more\n");
    }

  }


  //buf[i]
  //printf("%s\n", buf);
  //printf("\nmy read: read %d char from fd %d\n", desired, fdesc);
  return desired;
}

int write_file_wrap(char *fde, char *stuff)
{
  int fdesc = 0, bytes = 0;

  if(fde == 0 || stuff == 0)
  {
    printf("need fd and #bytes to read\n");
    return -1;
  }

  fdesc = atoi(fde);

  if(fdesc == 0 && strcmp(fde, "0") != 0)
  {
    printf("invalid fd\n");
    return -1;
  }


  write_file(fdesc, stuff);
}

int write_file(int fdesc, char *stuff)
{
  mywrite_file(fdesc, stuff);
}

int mywrite_file(int fdesc, char stuff[])
{
  MINODE *mip;
  int avail = 0, remain = 0;
  int offset = 0, start = 0;
  int lbk = 0;
  char *cp;
  int more = 0, block_check = 0, i =0;
  int desired = 0;
  char buf[BLKSIZE+1];
  int nbytes = 0;

  bzero(data_buf, BLKSIZE);

  if(fdesc > NFD || fdesc < 0)
  {
    printf("fd to read is out of range\n");
    return -1;
  }

  if(running->fd[fdesc] == 0 || running->fd[fdesc]->refCount == 0)
  {
    printf("%d is not open\n", fdesc);
    return -1;
  }

  if(running->fd[fdesc]->mode != 1 && running->fd[fdesc]->mode != 2 && running->fd[fdesc]->mode != 3)
  {
    printf("file descriptor %d is not open for write (1 or 2)\n", fdesc);
    return -1;
  }

  //at this point our fd is open and open for read more
  mip = running->fd[fdesc]->inodeptr;
  offset = running->fd[fdesc]->offset;
  nbytes = strlen(stuff);

  if(nbytes <= 0)
  {
    printf("nothing to write\n");
  }

 /* if(offset == mip->INODE.i_size)
  {
    //allocate new block
    //printf("at end of file\n");pressEnterToContinue();
   
  }*/
  if(offset > mip->INODE.i_size)
  {
    printf("wtf, this shouldn't happen\n");
    return 0;
  }
  else
  {
    mip->dirty = 1;
  }

  //avail = mip->INODE.i_size - offset;
  lbk = offset / BLKSIZE;
  start = offset % BLKSIZE;
  remain = nbytes;
  desired = nbytes;

  remain = desired;

  bzero(data_buf, BLKSIZE);
  bzero(buf, BLKSIZE + 1);
  more = nextDataBlock(&(mip->INODE), lbk++);

  if(more == 0)
  {
    more = newDataBlock(mip);
    printf("need new datablock\n");
  }

  get_block(mip->dev, more, data_buf);
  cp = data_buf;
  cp+= start;


  if(remain >= BLKSIZE - start)
  {
      memcpy(cp, &stuff[i], BLKSIZE - start);
      (running->fd[fdesc]->offset) += (BLKSIZE - start);
      i+= (BLKSIZE - start);
      remain-=(BLKSIZE - start);
      mip->INODE.i_size+=i;
      put_block(mip->dev, more, data_buf);   
  }
  else
  {
    memcpy(cp, &stuff[i], remain);

    running->fd[fdesc]->offset += remain;
    i+=remain;
    remain = 0;  
    put_block(mip->dev, more, data_buf); 
    mip->INODE.i_size+=i;
    printf("-->wrote %d char to fd %d<--\n", i, fdesc);;
    return desired;
  }

  //printf("%s", buf);

  while(remain)
  {
    bzero(buf, BLKSIZE);

    more = nextDataBlock(&(mip->INODE), lbk++);

    if(more == 0)
    {
      more = newDataBlock(mip);
      printf("need new datablock\n");
    }

    get_block(mip->dev, more, data_buf);

    //i dont need to point cp i couldve just used data_buf
    cp = data_buf;

    if(remain >= BLKSIZE)
    {
      memcpy(cp, &stuff[i], BLKSIZE);
      running->fd[fdesc]->offset += BLKSIZE;
      i+=BLKSIZE;
      remain-=BLKSIZE;
      mip->INODE.i_size+=BLKSIZE;
      put_block(mip->dev, more, data_buf);
    }
    else
    {
      memcpy(cp, &stuff[i], remain);
      running->fd[fdesc]->offset += remain;
      i+=remain;
      mip->INODE.i_size+=remain;
      put_block(mip->dev, more, data_buf);
      remain = 0;
    }
  }


  //buf[i]
  //printf("%s\n", buf);
  printf("-->wrote %d char to fd %d<--\n", i, fdesc);
  return desired;
}

int copy_file_wrap(char *source, char *dest)
{
  int fdsrc = 0, fddes = 0, destcheck = 0;

  if(source == 0 || dest == 0)
  {
    printf("need source and dest to cpy\n");
    return -1;
  }

  while(strlen(source) > 1 && source[strlen(source) - 1] == '/')
  {
    source[strlen(source) - 1] = 0;
  }

  fdsrc = open_file(source, 0);

  if(fdsrc < 0)
  {
    printf("can't open file to copy\n");
    return -1;
  }

   while(strlen(dest) > 1 && dest[strlen(dest) - 1] == '/')
  {
    dest[strlen(dest) - 1] = 0;
  }

  destcheck = creato(dest);
  //printf("dest check: %d",destcheck);getchar();

  if(destcheck < 0)
  {
    //try to unlink
    destcheck = unlink(dest);

    if(destcheck)
    {
      close_file(fdsrc);
      printf("can't creat file to copy to\n");
      return -1;
    }
    else{
      printf("removing previous file\n");getchar();
    }

    destcheck = creato(dest);

    if (destcheck < 0)
    {
      printf("this is some bullshit how could this happen. i was unlinked therefore it should be able to be made now\n");
      return -1;
    }
  }
  
  //copy_file(fdsrc, fddes);

  fddes = open_file(dest, 1);

  copy_file(fdsrc, fddes);
  close_file(fdsrc);
  close_file(fddes);

  return 0;
}

int copy_file(int fdsource, int fddest)
{
  mycopy_file(fdsource, fddest);
}

int mycopy_file(int fdsource, int fddest)
{
  
  MINODE *mip;
  int avail = 0, remain = 0;
  int offset = 0, start = 0;
  int lbk = 0;
  char *cp;
  int more = 0, block_check = 0, i =0;
  int desired = 0;
  char buf[BLKSIZE+1];

  bzero(data_buf, BLKSIZE);
  if(fdsource > NFD || fdsource < 0)
  {
    printf("fd to read is out of range\n");
    return -1;
  }

  if(running->fd[fdsource] == 0 || running->fd[fdsource]->refCount == 0)
  {
    printf("%d is not open\n", fdsource);
    return -1;
  }

  if(running->fd[fdsource]->mode != 0 && running->fd[fdsource]->mode != 2)
  {
    printf("file descriptor %d is not open for read (0 or 2)\n", fdsource);
    return -1;
  }

  //at this point our fd is open and open for read more
  mip = running->fd[fdsource]->inodeptr;
  offset = running->fd[fdsource]->offset;

  if(offset == mip->INODE.i_size)
  {
    printf("no more to read\n");
    return 0;
  }
  if(offset > mip->INODE.i_size)
  {
    printf("no more to read. how tf did this happen?\n");
    return 0;
  }

  avail = mip->INODE.i_size - offset;
  lbk = offset / BLKSIZE;
  start = offset % BLKSIZE;
  remain = avail;
  desired = avail;

  //this should happen while copying , if it does that's some bullshit
  if(desired > avail)
  {
    printf("you want to read %d, but only %d available.", desired, avail);pressEnterToContinue();
    desired = avail;
  }

  //redundant af but w/e im using a lot of copying an pasting from other functions this is harmeless to me
  remain = desired;

  bzero(data_buf, BLKSIZE);
  bzero(buf, BLKSIZE + 1);
  more = nextDataBlock(&(mip->INODE), lbk++);
  get_block(mip->dev, more, data_buf);
  cp = data_buf;
  cp+= start;
  //printf("%d\n", start);getchar();

  //getting first logical block
  if(remain >= BLKSIZE - start)
  {
      memcpy(buf, cp, (data_buf + BLKSIZE) - cp );
      (running->fd[fdsource]->offset) += (BLKSIZE - start);
      remain-=(BLKSIZE - start);
      mywrite_file(fddest, buf);   
  }
  else
  {
    memcpy(buf, cp, remain);
    running->fd[fdsource]->offset += remain;
    remain = 0;  
    mywrite_file(fddest, buf);
    printf("\nmy read: read %d char from fd %d\n", desired, fdsource);
    return desired;
  }

  //printf("%s", buf);

  more = nextDataBlock(&(mip->INODE), lbk++);
  //getchar();

  while(more && remain)
  {
    bzero(buf, BLKSIZE);

    //printf("-->more: %d<--", more);getchar();
    get_block(mip->dev, more, data_buf);

    if(remain >= BLKSIZE)
    {
      memcpy(buf, data_buf, BLKSIZE);
      running->fd[fdsource]->offset += BLKSIZE;
      remain-=BLKSIZE;
    }
    else
    {
      memcpy(buf, data_buf, remain);
      running->fd[fdsource]->offset += remain;
      remain = 0;
    }

    more = nextDataBlock(&(mip->INODE), lbk++);
    mywrite_file(fddest, buf);

    if(!more && remain)
    {
      printf("wtf no more blocks to read but we need more\n");
    }

  }


  //buf[i]
  //printf("%s\n", buf);
  printf("\nmy cpy: cpyd %d char\n", desired);
  return desired;
}

int move_file_wrap(char *source, char *dest)
{
  int check = copy_file_wrap(source, dest);

  if(check < 0)
  {
    printf("move not complete\n");
  }

  unlink(source);
}

#endif


