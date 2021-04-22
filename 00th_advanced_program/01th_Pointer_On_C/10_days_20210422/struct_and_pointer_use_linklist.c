#include "stdio.h"
#include "stdlib.h"

static typedef struct node_link {
	struct node_link *next;
	int value;
} Node;

static int insert_node( Node **rootp, int new_value)
{
	Node *pre, *cru, *new;
	cru = *rootp;
	pre = NULL;
	while( cru != NULL && cru->value < new_value)
	{
		pre = cru;
		cru = cru->next;
	}

	new = (Node *)malloc(sizeof(Node));
	if(new == NULL)
		return -1;
	new->next = cru;
	new->value = new_value;
	if(pre == NULL)
	{
		*rootp = new;
	}
	else
	{
		pre->next = new;
	}

	return 0
}
