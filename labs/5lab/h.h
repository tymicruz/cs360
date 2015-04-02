/******* ialloc.c: allocate a free INODE, return its inode number ******/
#ifndef HH
#define HH

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
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

	printf("SUPER BLOCK content at 0th block:\n");
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
  	printf("s_wtime \t\t= %s\n", (char *)ctime(&sp->s_wtime));

	//find how many inodes there are per block
	inodes_per_block_G = BLKSIZE/(sp->s_inode_size);
	num_inode_blocks_G = (sp->s_inodes_count)/inodes_per_block_G;

	if((sp->s_inodes_count)%inodes_per_block_G!=0){num_inode_blocks_G++;}

}

//call this in init
getGDInfo(int dev)
{
	get_block(dev, GDBLOCK, group_buf);
	gp = (GD*)group_buf;

	imap_block_G = gp->bg_inode_bitmap;
	bmap_block_G = gp->bg_block_bitmap;  
	inode_begin_block_G = gp->bg_inode_table;
}

int init(int device)
{
	superCheck(device);
	getGDInfo(device);
}



#endif


