#include "stdio.h"
#include "stdlib.h"

static typedef struct link_tag {

	struct link_tag *fwd;
	struct link_tag *bwd;
	int value;
} Node;


static db_insert(Node *rootp, int new_value)
{
	Node *this, *next, *newnode;

	for( this = rootp; (next = this->fwd) != NULL; this = next)
	{
		if(next->value  == new_value)
			return 0;
		if(next->value > new_value)
		{
			break;
		}
	}
	newnode = (Node *)malloc(sizeof(Node));

	if(newnode == NULL)
		return -1;

	newnode->value = newnode;


	newnode->fwd = this;
	newnode->bwd = 
}
