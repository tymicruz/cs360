#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>	
#include <linux/ext2_fs.h>

#define BLKSIZE 1024
#define MAX_PATH_LEN 256
#define MAX_PATH_PIECES 64


//define short TYPE, save typing efforts (note from Dr. K.C. Wang)
typedef struct ext2_group_desc 	GD;
typedef struct ext2_super_block 	SUPER;
typedef struct ext2_inode 	INODE;
typedef struct ext2_dir_entry_2 	DIR;

GD 	*gp;
SUPER *sp;
INODE *ip;
DIR 	*dp;

char *cp;

char super_buf[BLKSIZE];
char group_buf[BLKSIZE];
char inode_buf[BLKSIZE];
char data_buf[BLKSIZE];
char buf[BLKSIZE];

int fd;

int InodesBeginBlock = 0;
int BmapBlockNumber = 0;
int ImapBlockNumber = 0;

int inodes_per_block = 0;
int num_inode_blocks = 0;



int get_block(int fd, int blk, char buf[])
{
	lseek(fd, (long)blk*BLKSIZE, 0);
	read(fd, buf, BLKSIZE);
}

int superCheck()
{
	get_block(fd, 1, super_buf);//super block is the 1th block
	//buf now holds a SUPER struct

	sp = (SUPER *)super_buf;
	//(SUPER *) is just telling compiler to shut up
	//(SUPER *) is unnecessary

	//is this an EXT2 filesystem?
	//let's check the magic number
	printf("the SUPER s_magic num of this disk image is 0x%04x.\n", sp->s_magic);
	printf("the magic number for an EXT2 file system is 0xEF53.\n");

	if(sp->s_magic != 0xEF53)
	{
		printf("NOT an EXT2 file system\n");
		printf("exiting program ..\n");
		exit(1);
	}
	
	printf("Great, they match!\n");
	printf("Therefore, this disk image is an EXT2 file system.\n\n");

	printf("SUPER BLOCK content at 0th block:\n");
  	printf("s_inodes_count = %d\n", sp->s_inodes_count);
  	printf("s_blocks_count = %d\n", sp->s_blocks_count);

  	printf("s_free_inodes_count = %d\n", sp->s_free_inodes_count);
  	printf("s_free_blocks_count = %d\n", sp->s_free_blocks_count);
  	printf("s_first_data_blcok = %d\n", sp->s_first_data_block);
	
  	printf("s_log_block_size = %d\n", sp->s_log_block_size);
  	printf("s_log_frag_size = %d\n", sp->s_log_frag_size);

  	printf("s_blocks_per_group = %d\n", sp->s_blocks_per_group);
  	printf("s_frags_per_group = %d\n", sp->s_frags_per_group);
  	printf("s_inodes_per_group = %d\n", sp->s_inodes_per_group);


  	printf("s_mnt_count = %d\n", sp->s_mnt_count);
  	printf("s_max_mnt_count = %d\n", sp->s_max_mnt_count);

  	printf("s_magic = %x\n", sp->s_magic);
	printf("s_inode_size = %d\n", sp->s_inode_size);

  	printf("s_mtime = %s", (char *)ctime(&sp->s_mtime));
  	printf("s_wtime = %s\n", (char *)ctime(&sp->s_wtime));

}

char* addNullToDirName(char* name, int len)
{
	char* return_name = (char*)malloc(sizeof(char)*(len+1));
	int i = 0;

	for(i = 0; i < len; i++)
	{
		return_name[i] = name[i];
	}

	return_name[i] = 0;

	return return_name;
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
	*block_num = ((inode - 1) / inodes_per_block) + InodesBeginBlock;
	*offset = (inode - 1) % inodes_per_block;
}

int main(int argc, char* argv[], char* env[])
{
	
	char *path;
	char dp_name[32];
	char path_copy[MAX_PATH_LEN];
	char *path_pieces[MAX_PATH_PIECES];

	int path_count = 0, i = 0, j = 0;
	int search_count = 0;
	int block_num, offset;
	int found = 0, no_hope = 0;
	int blocks = 0;
	char temp;

	char current_directory[128] = "/";

	if(argc < 2)
	{
		printf("need a device image\n");
		printf("exiting program ..\n");
		exit(1);
	}

	fd = open(argv[1], O_RDONLY);

	if(fd < 0)
	{
		printf("open failed\n");
		printf("exiting program ..\n");
		exit(1);
	}

	//will exit in super() function if given disk image is not
	//an EXT2 file system
	superCheck();
	inodes_per_block = BLKSIZE/(sp->s_inode_size);
	num_inode_blocks = sp->s_inodes_count / inodes_per_block;
	
	printf("s_inodes_count = %d\n", sp->s_inodes_count);
	printf("inodes_per_block = (BLKSIZE) / (inode struct size) = (%d) / (%d) = %d\n",
	BLKSIZE, sp->s_inode_size, inodes_per_block);
	printf("num_inode_blocks = %d\n\n", num_inode_blocks);

	if(argc < 3)
	{
		printf("need a path\n");
		printf("exiting program ..\n");
		exit(1);
	}

	//printf("path len: %d\n", strlen(argv[2]));

	//"stack smashing" will occur if path length is greater than 128 chars
	if(strlen(argv[2]) > MAX_PATH_LEN)
	{
		printf("given path is too long, max 128\n");
		printf("exiting program ..\n");
		exit(1);
	}
	strcpy(path_copy, argv[2]);

	path = strtok(path_copy, "/");

	while(path != NULL)
	{
		path_pieces[path_count] = path;
		path_count++;
		path = strtok(NULL, "/");

		if(path_count - 1 == MAX_PATH_PIECES)
		{
			printf("too many folder redirections, 31 max\n");
			printf("exiting program ..\n");
		
			exit(1);
		}
	}

	path_pieces[path_count] = 0; //null terminate the pieces arrary
	printf("total path: %s\n", argv[2]);

	for(i = 0; i < path_count; i++)
	{
		printf("piece %d: %s\n", i, path_pieces[i]);
	}

	//read group descripter into buffer
	get_block(fd, 2, group_buf);

	gp = (GD *)group_buf;

	//use group descriptor pointer to find these values
	BmapBlockNumber = gp->bg_block_bitmap;
	ImapBlockNumber = gp->bg_inode_bitmap;
	InodesBeginBlock = gp->bg_inode_table;

	printf("\nbmapblocknumber = %d\n", BmapBlockNumber);
	printf("imapblocknumber = %d\n", ImapBlockNumber);
	printf("inodesbeginblock = %d\n", InodesBeginBlock);

	get_block(fd, InodesBeginBlock, inode_buf);
	//getting first inode
	ip = (INODE *)inode_buf + 1;//get first inode which is /?
  	printf("\n--------------------root directory--------------------\n");
	printf("mode = %4x\n", ip->i_mode);

  	printf("size = %d bytes\n", ip->i_size);
  	printf("time = %s\n", ctime(&ip->i_ctime));

  	printf("i_block[0]=%d\n\n", ip->i_block[0]);	
	
	get_block(fd, ip->i_block[0], data_buf);
	dp = (DIR *)data_buf;
	cp = data_buf;
 	printf("root dir content:\t");
	printf("inode\treclen\tftype\tname\n");
	while(cp < (data_buf + BLKSIZE))
	{
		//add null to dp->name
		//strcpy(dp_name, addNullToDirName(dp->name, dp->name_len));


		printf("\t\t\t%-5d\t%-6d\t%-5d\t", 
		dp->inode, dp->rec_len, dp->file_type);

		temp = dp->name[(dp->name_len)];
		dp->name[(dp->name_len)] = 0;
		
		printf("%s\n", dp->name);

		dp->name[(dp->name_len)] = temp;

		cp+=(dp->rec_len);
		dp = (DIR *)cp;
	}
	printf("\n------------------------------------------------------\n");

	printf("Let's start the inode search! Press ENTER to continue\n");
	pressEnterToContinue();

	//we are going to start with the root dir
	while(search_count < path_count && !no_hope && !found)
	{
		dp = (DIR *)data_buf;
		cp = data_buf;
		printf("\nLooking for piece%d (%s) of %s in %s\n", search_count, path_pieces[search_count], argv[2], current_directory);
		printf("inode\treclen\tftype\tname\n");

		while((cp < (data_buf + BLKSIZE)))
		{
			//add null to dp->name
			strcpy(dp_name, addNullToDirName(dp->name, dp->name_len));	
			printf("%-5d\t%-6d\t%-5d\t%s\n", 
			dp->inode, dp->rec_len, dp->file_type, dp_name);
		
			if(strcmp(path_pieces[search_count], dp_name) == 0)
			{
				search_count++;

				if(search_count < path_count)//keep looking
				{
					strcat(dp_name, "/");
					strcat(current_directory, dp_name);

					//get inode of this thing we just found
					getInodeBlockNumberAndOffset(dp->inode, &block_num, &offset);
					get_block(fd, block_num, inode_buf);
					ip = (INODE *)inode_buf + offset;


					if(!S_ISDIR(ip->i_mode))//cant search anymore because this isn't a dir
					{
						no_hope = 1;
						printf("can't go any farther because this isn't a directory\n");
						break;
					}
				
					//get next inode location
					//getInodeBlockNumberAndOffset(dp->inode, &block_num, &offset);
					//get_block(fd, block_num, inode_buf);
				
					//find inode of this match
					//ip = (INODE *)inode_buf + offset;
					get_block(fd, ip->i_block[0], data_buf);
					printf("found %s directory, look for next piece of path.. ENTER", path_pieces[search_count-1]);
					pressEnterToContinue();
				}
				else
				{
					getInodeBlockNumberAndOffset(dp->inode, &block_num, &offset);
					get_block(fd, block_num, inode_buf);
					ip = (INODE *)inode_buf + offset;

					found = 1;
				}	

				break;
			}

			cp+=(dp->rec_len);
			dp = (DIR *)cp;

			if(cp >= (data_buf + BLKSIZE))//went through whole dir and we didn't find match
			{
				no_hope = 1;
				break;
			}
		}
	}

	if(no_hope)
	{
		printf("not_found\n");
		//ip now points to desired inode!
	}

	if(found)
	{
		printf("found\n");
		printf("size=%d\n", ip->i_size);
		if(ip->i_size % BLKSIZE)
		{
			blocks++;
		}
		blocks += ip->i_size / BLKSIZE;
		printf("blocks: %d\n", blocks);
		printf("\n------------direct blocks------------\n");
		for(i = 0; i < 12 && i < blocks; i++)
		{
			printf("%d\n", ip->i_block[i]);
		}

		if(i >= blocks)
		{
			return 0;
		}

		printf("blocks left: %d\n", blocks - i);
		blocks-= i;
		printf("\n----------------indirect blocks---------------\n");

		printf("%d\n", ip->i_block[12]);
		j = ip->i_block[12] + 1;	
		for(i = 0; i < 256 && i < blocks;i++)
		{
		
			if(i % 10 == 0) printf("\n");
			printf("%-4d ", j);
			j++; 
		}

		if(i >= blocks)
		{
			return 0;
		}

		printf("\nblocks left: %d\n", blocks - i);
		blocks-= i;


		printf("\n------------------double indirect blocks---------------\n");

		printf("%d\n", ip->i_block[13]);
		j = ip->i_block[13] + 1;


		for(i = 0; i < 256*256 && i < blocks;i++)
		{
			
			if(i % 10 == 0) printf("\n");

			printf("%-4d ", j);
			j++; 
		}
		printf("blocks left: %d\n", blocks - i);
		blocks-= i;

		
		printf("\n");
	}
}
