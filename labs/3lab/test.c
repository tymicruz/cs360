#include <stdio.h>
#include <errno.h>
typedef struct node{
int num;
struct node *pNext;
}Node;

int addToFront(Node **pHead,int num)
{
	Node *newNode = (Node*)malloc(sizeof(Node));
	newNode->num = num;
	newNode->pNext = 0;

	if((*pHead) == 0)//empty
	{
		*pHead = newNode;
	}else
	{
		newNode->pNext = *pHead;
		*pHead = newNode;
	}
}

int printList(Node *node)
{
	if(node)
	{
		printf("%d->\n", node->num);
		node = node->pNext;
		printList(node);
	}
}

int main (int argc, char *argv[], char* env[])
{
	int r;
	Node *pHead = 0;	
	addToFront(&pHead, 1);
	addToFront(&pHead, 2);
	addToFront(&pHead, 3);
	addToFront(&pHead, 4);

	printList(pHead);
	
	r = execve("bla.out", argv, env);

	printf("FAILLLLLLLL\n");
	printf("errno: %d / strerror: %s\n", errno, strerror(errno));
}
