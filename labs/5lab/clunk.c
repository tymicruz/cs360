#include "h.h"
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

