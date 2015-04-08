#include "h2.h"

int get_block(int dev, int blk, char buf[ ]);
int put_block(int dev, int blk, char buf[ ]);
int tst_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
int clr_bit(char *buf, int bit);
int getInodeBlockNumberAndOffset(int inode, int *block_num, int *offset);
int superCheck(int dev);
getGDInfo(int dev);
int init();
int getino(int dev, char *pathname);
MINODE* iget(int dev, int ino);
int iput (MINODE *mip);
int findmyname(MINODE *parent, int myino, char *myname);
int findino(MINODE *mip, int *myino, int *parentino);
int mount_root();
int list_file(MINODE *mip, char *name);
int list_dir(MINODE *mip);
ls(char *pathname);
ls_wrap(char *path);
cd(char *path);
int cd_wrap(char *path);
char* pwd();
int tokenize(char *path, char *pieces[], int *npieces);
int tokCmd(char *line, char *myargv[], int *myargc);
int nextDataBlock(INODE *ip, int num);
int quit();
int get_block(int dev, int blk, char buf[ ])
{

	lseek(dev, (long)blk*BLKSIZE, 0);
	read(dev, buf, BLKSIZE);
}

int put_block(int dev, int blk, char buf[ ])
{

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

int pressEnterToContinue(){
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
	printf("num inodes \t\t= %d\n", num_inodes_G);
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

int init()
{
	int i = 0;

	running = (PROC*)malloc(sizeof(PROC));

	p0;// = (PROC*)malloc(sizeof(PROC));
	p1;// = (PROC*)malloc(sizeof(PROC));


	//not sure about the 5 lines of initialization below
	running = &p1;

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
	
	//i = getino(device, "X"); 

	//printf("%d\n", i);

	//root = iget(device, i);
	//printf("%x\n", minode[0].INODE.i_mode);

	//printf("%d\n", minode[0].ino);
	//printf("%d\n", root->ino);	
}

//only goes up to doubley indirect blocks

//this takes a full path from root
int getino(int dev, char *pathname)
{
	char *path_pieces[MAX_PATH_PIECES];
	int path_count = 0, search_count = 0;
	int block_num = 0, offset = 0;

	int found = 0, no_hope = 0;
	char *cp, temp = 0;
	int i = 0;
	int more = 0;
	int block_check = 0;

	tokenize(pathname, path_pieces, &path_count);

	if(path_count == 0)
	{
		return ROOT_INODE;
	}

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
			//printf("%s\n", dp->name);//getchar();
			//dir name match //
			if(strcmp(path_pieces[search_count], dp->name) == 0)
			{
				search_count++;
				//printf("match:%s, %d more to go\n", dp->name, path_count - search_count);//getchar();
				if(search_count < path_count)//keep looking, not at end of path
				{
					//strcat(current_directory, dp->name);
					//strcat(current_directory, "/");

					dp->name[dp->name_len] = temp;//put char back that we moved
					
					//get inode of this thing we just found
					getInodeBlockNumberAndOffset(dp->inode, &block_num, &offset);
					get_block(fd, block_num, inode_buf);

					//now we have inode of current path we've seen so far
					ip = (INODE *)inode_buf + offset;

					if(!S_ISDIR(ip->i_mode))//cant search anymore because this isn't a dir
					{
						no_hope = 1;
						//printf("path is at dead end cuz we at a file\n");
						return 0;
						//break out of inner loop and then will break out of outer because no_hope = 1;
						break;
					}
				
					//if match a directory, we go here
				
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

					return dp->inode;
					found = 1;
				}	

				//WE WILL BREAK IF WE MATCH TO NEXT ITERATION
				break;
			}
			else
			{
				//MUST PUT THE CHAR BACK 	~!!!!!!!!! SUCH A STUPUD BUG
				dp->name[dp->name_len] = temp;
			}


			cp+=(dp->rec_len);
			dp = (DIR *)cp;

			//if we have looked through the end of the dir and we haven't seen the name
			if(cp >= (data_buf + BLKSIZE) && !more)//went through whole dir and we didn't find match
			{
				printf("end of buf, and we can't find\n");
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

//need to make sure we are getting a node that exists
MINODE* iget(int dev, int ino)
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
			minode[i].refCount = 1;

			getInodeBlockNumberAndOffset(ino, &block_num, &offset);
			get_block(dev, block_num, inode_buf);
			ip = (INODE*)inode_buf + offset;

			//copy ip node to inmemoryinode	
			memcpy(&(minode[i].INODE), ip, sizeof(INODE));
		//printf("can't find it");getchar();
			return &(minode[i]);
		}
		else if(minode[i].ino == ino){
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

	//in memory inode is same as on disk so we don't need to put anything
	if((mip->dirty) == 0)
	{
		//should already be 0,
		//but if we pass in an MINODE with ref count 0 we could 
		//make refCount Negative, so to be safe we'll set it to 0
		mip->refCount = 0; //make sure refCount is 0 and not negative
		return;
	}
	
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
	if (parent == 0){return 0;}

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
				get_block(fd, more, data_buf);	
	
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
			//printf("%d cp:%d data_buff:%d dp->name_len:%d\n", more, cp, data_buf + BLKSIZE, dp->name_len);
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

int mount_root()
{
	int me, parent;
	char myname[32];

	
	fd = open(disk, O_RDWR);

	if(fd < 0)
	{
		printf("couldn't open %s\n", disk);
		exit(1);
	}

	superCheck(fd);
	getGDInfo(fd);

	root = iget(fd, ROOT_INODE);
 	p0.cwd = iget(fd, ROOT_INODE); 
 	p1.cwd = iget(fd, ROOT_INODE);

	printf("root minode refCount = %d\n", root->refCount);
	
	//(char *)ctime(&ip->i_ctime)
	running->cwd = iget(fd, ROOT_INODE);
	//printf("%d", (char*)ctime(running->cwd->INODE.i_ctime));
	//list_dir(running->cwd);
	//ls("./");
}

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

  /*for (i=8; i >= 0; i--){
		//if bit is 1
    if (ip->i_mode & (1 << i))
		{
			printf("%c", t1[i]);
		}	
    else
		{
			printf("%c", t2[i]);
		}
  }*/

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

			cip = iget(mip->dev, dp->inode);
			list_file(cip, dp->name);

			dp->name[dp->name_len] = temp;
			
			iput(cip);
			cp+=dp->rec_len;
			dp=(DIR*)cp;
		}
		
		more = nextDataBlock(&(mip->INODE), block_check++);
		
	}
}

ls(char *pathname)
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

	if(S_ISREG(mip->INODE.i_mode))
	{
		list_file(mip, (char *)basename(pathname));
	}
	else
	{
		list_dir(mip);
	}

	iput(mip);
}

ls_wrap(char *path)
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

cd(char *path)
{
	MINODE *mip;
	int ino;
			
	if(0 == path)
	{
		mip = iget(fd, 2);
		iput(running->cwd);
		printf("*cd to root*\n");
		running->cwd = mip;
		return;
		//cd to rood
	}

	ino = getino(fd, path);

	//if ino = 0 return, we d
	if(!ino) 
	{
		printf("can't cd to that garbage\n");
		return;
	}

	mip = iget(fd, ino);

	//if inode doesn't exit
	if(!S_ISDIR(mip->INODE.i_mode))
	{
		iput(mip);
		printf("can't cd to a non-directory file\n");
		return;
	}
	
	//file is a directory at this point

	//iput minode of running proc

	//set running cwd to in memory inode that we just got 
	running->cwd = mip;
	printf("cd to %s success\n", pwd());
	//printf("cd succes\n");
}

int cd_wrap(char *path)
{
	char cdpath[MAX_PATH_LEN] = "";

	if(path == 0)
	{
		cd(0);//change dir to root
		return;
	}
	//we'll ls from root
	if(path[0] == '/')
	{
		cd(path);
		return;
	}

	//go from current dir
	strcat(cdpath, pwd());
	strcat(cdpath, "/");
	strcat(cdpath, path);

	//change dir from current working directory

	//printf("try to cd to this:%s\n", cdpath);

	cd(cdpath);
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

	while(strlen(cp))
	{
		strcat(path, "/");
		strcat((char*)path, cp);
		//printf("--<>%s<>--\n", cp);
		rpath[strlen((char*)rpath) - strlen((char*)basename(rpath)) - 1] = 0;
		cp = (char*)basename(rpath);
	}
	//printf("-->%s<--\n", path);
	return (char*)path;
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
			/*if((*intp) == 0)
			{
				return 0;
			}*/

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

	exit(1);

}

