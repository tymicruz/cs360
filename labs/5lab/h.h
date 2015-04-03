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

PROC *running, p0, p1;
MINODE minode[NMINODES], *root;

int nextDataBlock(INODE *ip, int num);
MINODE* iget(int d, int i);

int get_block(int dev, int blk, char buf[ ]){

	lseek(dev, (long)blk*BLKSIZE, 0);
	read(dev, buf, BLKSIZE);
}

int put_block(int dev, int blk, char buf[ ]){

	lseek(dev, (long)blk*BLKSIZE, 0);
	write(dev, buf, BLKSIZE);
}

int tst_bit(char *buf, int bit)
{
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

int set_bit(char *buf, int bit)
{
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

int clr_bit(char *buf, int bit)
{
	int byte = bit/8;
	int bit_int_byte = bit%8;

	//bit is already clr
	if(tst_bit(buf, bit)==0)
	{
		return -1;
	}

	//xor the bit you want to clr
	//if target bit is 1 it will set to 0
	//if target bit is 0 it will be set to 0
	//untargeted bits:	if 1, bit will stay 1 (1 xor 0 is 1)
	//			if 0, bit will stay 0 (0 xor 0 is 1)

	//xor targeted bit with binary 1
	buf[byte] = buf[byte] ^ (1 << bit_int_byte);
	
	//return cleared bit
	return bit;
} 

int pressEnterToContinue()
{
	char line[2];
	__fpurge(stdin);
	fgets(line, 2, stdin);
	__fpurge(stdin);
}

int getInodeBlockNumberAndOffset(int inode, int *block_num, int *offset)
{
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

	num_inodes_G = inodes_per_block_G * num_inode_blocks_G;
	printf("inodes per block \t= %d\n", inodes_per_block_G);
	printf("num inode blocks \t= %d\n", num_inode_blocks_G);
	printf("num inodes \t= %d\n", num_inodes_G);

}

//call this in init
getGDInfo(int dev)
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

int init(int device)
{
	int i = 0;
	running = (PROC*)malloc(sizeof(PROC));

	p0;// = (PROC*)malloc(sizeof(PROC));
	p1;// = (PROC*)malloc(sizeof(PROC));

	superCheck(device);
	getGDInfo(device);

	
	//not sure about the 5 lines of initialization below
	*running = p1;

	p0.uid = 0;
	*(p0.fd) = 0;

	p1.uid = 1;
	*(p1.fd) = 0;

	//init all minodes
	for(i = 0; i < NMINODES; i++)
	{	
		//printf("%d\n",i);
		minode[i].refCount=0;
		minode[i].dev=0;
		minode[i].ino=0;
	}

	//root is a minode pointer
	
	root = 0;
	//root = (MINODE*)malloc(sizeof(MINODE));
	
	i = getino(device, "X"); 

	//printf("%d\n", i);

	root = iget(device, i);
	printf("%x\n", minode[0].INODE.i_mode);

	printf("%d\n", minode[0].ino);
	printf("%d\n", root->ino);
	
}

//only goes up to doubley indirect blocks


//this takes a full path from root
int getino(int dev, char *pathname)
{
	char *path_pieces[MAX_PATH_PIECES];
	char current_directory[MAX_PATH_LEN] = "/";
	int path_count = 0, search_count = 0;
	int block_num, offset;

	int found = 0, no_hope = 0;
	char *cp, temp;
	int i = 0;
	int more = 0;
	int block_check = 0;
	
	tokenize(pathname, path_pieces, &path_count);

	//load first inode block into memory so we can get root
	get_block(fd, inode_begin_block_G, inode_buf);

	ip = (INODE *)inode_buf + 1;//get first inode which is root
	//load data block of root inode
	get_block(fd, ip->i_block[block_check], data_buf);	
	
	dp = (DIR *)data_buf;
	cp = data_buf;//same as cp = data_buf

	more = nextDataBlock(ip, block_check++);


	while(search_count < path_count && !no_hope && !found && more)
	{
		dp = (DIR *)data_buf;
		cp = data_buf;
		more = nextDataBlock(ip, block_check++);

		while((cp < (data_buf + BLKSIZE)))
		{
			//null terminate the dp->name field since there is no null char
			temp = dp->name[dp->name_len];
			dp->name[dp->name_len] = 0;
			//printf("%s", dp->name);getchar();
			//dir name match //
			if(strcmp(path_pieces[search_count], dp->name) == 0)
			{
				search_count++;

				if(search_count < path_count)//keep looking, not at end of path
				{
					strcat(current_directory, dp->name);
					strcat(current_directory, "/");

					dp->name[dp->name_len] = temp;//put char back that we moved
					
					//get inode of this thing we just found
					getInodeBlockNumberAndOffset(dp->inode, &block_num, &offset);
					get_block(fd, block_num, inode_buf);

					//now we have inode of current path we've seen so far
					ip = (INODE *)inode_buf + offset;

					if(!S_ISDIR(ip->i_mode))//cant search anymore because this isn't a dir
					{
						no_hope = 1;
						//break out of inner loop and then will break out of outer because no_hope = 1;
						break;
					}
				
					//if match a directory, we go here
				
					//get first data block of this dir
					block_check = 0;
					get_block(fd, ip->i_block[block_check], data_buf);

					//GET MORE OF THIS SO WE CAN ENTER NEXT LOOP
					more = nextDataBlock(ip, block_check++);
				}
				else
				{
					//found
					dp->name[dp->name_len] = temp; //put char back that we moved
					//WE FOUND A MATCH IN THIS DIR
					//NO WE NEED TO GO TO THE INODE FROM WHAT WE SEE AT THE CURRENT DP POINTER
					getInodeBlockNumberAndOffset(dp->inode, &block_num, &offset);
					get_block(fd, block_num, inode_buf);
					ip = (INODE *)inode_buf + offset;

					return dp->inode;
					found = 1;
				}	

				//WE WILL BREAK IF WE MATCH TO NEXT ITERATION
				break;
			}

			cp+=(dp->rec_len);
			dp = (DIR *)cp;

			if(cp >= (data_buf + BLKSIZE) && !more)//went through whole dir and we didn't find match
			{
				no_hope = 1;
				return 0;
				break;
			}
			else if(cp >= (data_buf + BLKSIZE))
			{
				//check next data block if you didn't find it in this data block
				get_block(fd, more, data_buf);
				break;
			}
		}
	}

	//couldn't find dat ish
	return 0;
}

MINODE *iget(int dev, int ino)
{	
	int block_num = 0, offset = 0, i = 0;



	//inode is not in memory
	for(i = 0; i < NMINODES; i++)
	{
		//found an unused minode
		if(minode[i].refCount == 0)
		{
			minode[i].ino = ino;
			minode[i].dev = dev;
			minode[i].dirty = 0;

			getInodeBlockNumberAndOffset(ino, &block_num, &offset);
			get_block(dev, block_num, inode_buf);
			ip = (INODE*)inode_buf + offset;

			memcpy(&(minode[i].INODE), ip, sizeof(INODE));

			return &(minode[i]);
		}
		else if(minode[i].ino == ino)
		{
			minode[i].refCount++;

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
		return;

	if((mip->dirty) == 0)
	{
		mip->refCount = 0; //make sure refCount is 0 and not negative
		return;
	}

	//use ino to get inode block
	
		getInodeBlockNumberAndOffset(mip->ino, &block_num, &offset);
		get_block(mip->dev, block_num, inode_buf);
		ip = (INODE*)inode_buf + offset;

	//now it is implied that refCount is 0
	//we know refCount must now be 0; otherwise it must be negative which means something else messed up to have an imemory node with a negative refCount

	

}


































int tokenize(char *path, char *pieces[], int *npieces)
{
	char *p;
	char path_copy[MAX_PATH_LEN];

	*npieces = 0;

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
		pieces[(*npieces)++] = p;
		p = strtok(NULL, "/");

		if((*npieces) - 1 == MAX_PATH_PIECES)
		{
			printf("too many folder redirections, %d max\n", MAX_PATH_PIECES - 1);
			printf("exiting program ..\n");
		
			exit(1);
		}
	}

	pieces[(*npieces)] = 0;//null terminate array of strings
}


































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

#endif


