#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio_ext.h>//__fpurge() function
#define N 100
 
typedef struct proc{
	struct proc *next;
	char name[64];
	int  id;
	int priority;
}PROC;

PROC proc[N], *freeList, *readyQueue;

int init()
{
	int i;
	PROC *p;
	char name[64];//"proc" + max_2_digit_number = 7 chars + null char
	
	for(i = 0; i < N; i++)
	{
		p = &proc[i];
		sprintf(name, "%s%d", "proc", i); // node0, node1, node2 etc	
		strcpy(p->name, name);//source, destination
		p->id = i;
		p->priority = 0;
		if(i != (N-1))
		{
			p->next = p+1;        // node[i].next = &node[i+1];
		}
		else
		{
			p->next = 0;
		}
	}
	proc[N-1].next = 0;
	freeList = &proc[0];
	readyQueue = 0;

	return 0;
}
//TYLER CRUZ
int enqueue(PROC **queue, PROC *p)
{
	PROC *trail = 0;
	PROC *curr = *queue; //point to the first thing in queue
	if(p)
	{//if p is not null
		if(!(curr))
		{//if curr is NULL (queue is empty)
			*queue = p;
			return 0;
		}
		else if((p->priority) > (curr->priority))
		{//p is highest priority//insert@front
			p->next = *queue;
			*queue = p;
			return 0;
		}
		else
		{
			//we already checked first node in if block above, so skip it
			//at this point, we know there is at least one PROC node in the list
			trail = curr;
			curr = curr->next;

			while(curr && (p->priority <= curr->priority))
			{//while curr != NULL and priority is less than or equal to curr, keep looking
				
				trail = curr;//move trail pointer
				curr = curr->next;//move current pointer to next PROC node in the list
			}

			//set trail's next to p
			//set p(newnode) to curr
			trail->next = p;
			p->next = curr;
			//this handles inserting at the end (TYLER CRUZ) and any part inbetween
			//if curr is null, p(newnode) will be the last node and point to null, as the end should
			return 0;
		}
	}

	printf("(nothing was added)\n");
	return -1; //nothing was added
}

PROC *dequeue(PROC **queue)
{
	PROC *dProc = 0;

	if(*queue)
	{//if queue is not empty
		dProc = *queue;
		*queue = (*queue)->next;
		dProc->next = 0;
	}
	else
	{
		printf("(Can't dequeue from empty queue)\n");
	}
	
	//dProc will be 0 is queue was empty
	return dProc;
}

int printQueue(PROC *queue)
{
	if(queue == 0)
	{
		printf("queue is empty\n");
		return 0;
	}

	while(queue)
	{
		printf("%s (prior:%d id: %d)\n", (queue->name), (queue->priority), (queue->id));
		queue = queue->next;
	}

	return 0;
}


int changePriority(PROC **queue, char *name, int priority)
{
	PROC *trail = 0, *curr = *queue;

	if((curr) == 0)
	{//if queue is NULL (queue is empty)
		return 0;
	}
	else if(strcmp(curr->name, name) == 0)
	{//name match found on first PROC node in list
		//change priority of first PROC node
		curr->priority = priority;

		//remove curr from queue;
		*queue = (*queue)->next;
		curr->next = 0;

		//add curr back to queue
		enqueue(queue, curr);

		return 0;
	}
	else
	{//first node is not a match
	
		//we already handled first-node-match case in if block above, so skip it
		//at this point, we know there is at least one PROC node in the list
		trail = curr;
		curr = curr->next;

		while(curr && (strcmp(curr->name, name) != 0))
		{//while curr is not null and curr name does not match name, keep looking
			trail = curr;
			curr = curr->next;
		}

		if(curr == 0)
		{//curr is NULL //name match was not found in queue
			return -1;
		}
		else
		{//name match found
			trail->next = curr->next;//unlink curr from queue
			curr->next = 0;//curr is all alone now
			curr->priority = priority;//change priority of curr

			enqueue(queue, curr);//add curr back to list

			return 0; //or return enqueue(queue, curr);
		}
	}
}

int flushSHITINSTDIN()
{
//call this after every fgets call
//do what I expected fflush to do
//http://stackoverflow.com/questions/2187474/i-am-not-able-to-flush-stdin
	char c;
	fputc('x', stdin);
	while(((c = getchar()) != '\n') && c != EOF);
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

PROC *getNode()
{
	PROC *p;
	p = freeList;
	if (p)
	{
		freeList = freeList->next;
		p->next = 0;
	}
	return p;
 }
				 
int putNode(PROC *p)
{
	if(p)//if p is not empty add it to free list
	{
		p->next = freeList;
		freeList = p;
	}
}

int main(int argc, char *argv[], char *env[])
{
	char line[32];
	char *cmd;//command
	char *opt;//option
	char *opts[10] = {0};

	int i = 0;
	PROC *proc;
	srand(time(NULL));
	init();

	printf("\"done\" to terminate program\n");

	while(1)
	{
		i++;
	
		printf("\n(%d)cmd? ", i);
		fgets(line, 32, stdin);
		__fpurge(stdin);//same as fflush, but I think fflush is only for windows, while fpurge works for linux
		removeNewLine(line);

		if(strcmp(line, "") == 0) continue;

		cmd = strtok(line, " ");
		opt = strtok(NULL, " ");
		
		if(strcmp(cmd, "add") == 0)
		{
			printf("Before add:\n");
			printQueue(readyQueue);
			
			proc = getNode();//get PROC node from freeList
			
			if(proc)
			{
				proc->priority = rand() % 7;
				enqueue(&readyQueue, proc);

				printf("After add:\n");
				printQueue(readyQueue);
			}
			else
			{
				enqueue(&readyQueue, proc);

				printf("After add:\n");
				printQueue(readyQueue);
				printf("-->Nothing added (freeList used up)\n");
			}
		}
		else if(strcmp(cmd, "delete") == 0)
		{
			printf("Before delete:\n");
			printQueue(readyQueue);

			proc = dequeue(&readyQueue);

			if(proc)
			{//if proc is not NULL
				putNode(proc);//put dequeued node in the free list
			}
			printf("After delete:\n");
			printQueue(readyQueue);

		} 
		else if(strcmp(cmd, "chPriority") == 0)
		{
			if(opt)
			{
				printf("Before priority change:\n");
				printQueue(readyQueue);
				if(changePriority(&readyQueue, opt, rand() % 7) == -1)
				{
					printf("After priority change:\n");
					printQueue(readyQueue);
					printf("-->Name (%s) not found\n", opt);
				}
				else
				{
					printf("After priority change:\n");
					printQueue(readyQueue);
				}
			}
			else
			{
				printf("-->Need option for command (%s)\n", cmd);
			}
		}
		else if(strcmp(cmd, "done") == 0)
		{
			printf("program terminated\n");
 			break;
		}
		else
		{
			printf("-->Command (%s) not recognized\n", cmd);
		}
	
	}
	
	return 0;
}
