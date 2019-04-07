#include <stdio.h>
#include <stdlib.h>

typedef int item;

// ds don
typedef struct _basic
{
	item data;
	struct _basic *next;
} basic,*pbasic;

// ds kep
typedef struct _doubly
{
	item data;
	struct _doubly *left;
	struct _doubly *right;
} doubly,*pdoubly;

// ds vong
typedef struct _circular
{
	item data;
	struct _circular *next;
} circular,*pcircular;

// them gia tri
pbasic add_basic(item,pbasic);
pdoubly add_doubly(item,pdoubly);
pcircular add_circular(item,pcircular);

// sua gia tri
int edit_basic(item,item,pbasic);
int edit_doubly(item,item,pdoubly);
int edit_circular(item,item,pcircular);

// Xoa gia tri
pbasic remove_basic(item,pbasic);
pdoubly remove_doubly(item,pdoubly);
pcircular remove_circular(item,pcircular);

// show and return len
int show_basic(pbasic);		
int show_doubly(pdoubly);
int show_circular(pcircular);

pcircular tail;

int main()
{
	pbasic blist = NULL;
	pdoubly dlist = NULL;
	pcircular clist = NULL;
	
	int c,len;
	item data,value;
	
	while (1) {
		printf("############################\n");
		printf("1. Them\n");
		printf("2. Sua\n");
		printf("3. Xoa\n");
		printf("4. So luong phan tu\n");
		printf("5. Exit\n");
		printf("--> chon: ");
		scanf("%d",&c);
		printf("############################\n");
		
		switch (c)
		{
			case 1:		// Them gia tri
				printf("> Nhap so: ");
				scanf("%d",&data);
				printf("\t1. Ds don\n");
				printf("\t2. Ds kep\n");
				printf("\t3. Ds vong\n");
				printf("> chon: ");
				scanf("%d",&c);
				switch (c)
				{
					case 1:
						blist = add_basic(data,blist);
						break;
					case 2:
						dlist = add_doubly(data,dlist);
						break;
					case 3:
						clist = add_circular(data,clist);
						break;
					default:
						printf("Khong co chuc nang nay\n");
				}
				break;
			case 2:		// Sua gia tri
				printf("Nhap gia cu: ");
				scanf("%d",&data);
				printf("Nhap gia moi: ");
				scanf("%d",&value);
				printf("\t1. Ds don\n");
				printf("\t2. Ds kep\n");
				printf("\t3. Ds vong\n");
				printf("> chon: ");
				scanf("%d",&c);
				switch (c)
				{
					case 1:
						if (edit_basic(data,value,blist))
						{
							printf("Sua thanh cong\n");
						}
						else
						{
							printf("Sua khong thanh cong\n");
						}
						break;
					case 2:
						if (edit_doubly(data,value,dlist))
						{
							printf("Sua thanh cong\n");
						}
						else
						{
							printf("Sua khong thanh cong\n");
						}
						break;
					case 3:
						if (edit_circular(data,value,clist))
						{
							printf("Sua thanh cong\n");
						}
						else
						{
							printf("Sua khong thanh cong\n");
						}
						break;
					default:
						printf("Khong co chuc nang nay\n");
				}
				break;
			case 3:		// Xoa phan tu
				printf("> Nhap so: ");
				scanf("%d",&data);
				printf("\t1. Ds don\n");
				printf("\t2. Ds kep\n");
				printf("\t3. Ds vong\n");
				printf("> chon: ");
				scanf("%d",&c);
				switch (c)
				{
					case 1:
						blist=remove_basic(data,blist);
						show_basic(blist);
						break;
					case 2:
						dlist=remove_doubly(data,dlist);
						show_doubly(dlist);
						break;
					case 3:
						clist=remove_circular(data,clist);
						show_circular(clist);
						break;
					default:
						printf("Khong co chuc nang nay\n");
				}
				break;
			case 4:		// So luong
				printf("------------Don------------\n");
				printf("Len: %d\n",show_basic(blist));
				
				printf("------------Kep------------\n");
				printf("Len: %d\n",show_doubly(dlist));
				
				printf("------------Vong------------\n");
				printf("Len: %d\n",show_circular(clist));
				break;
			case 5:
				exit(0);
			default:
				printf("Khong co chuc nang nay\n");
		}
	}
}

pbasic add_basic(item data,pbasic list)
{
	pbasic node = (pbasic)malloc(sizeof(basic));
	
	node->data = data;
	node->next = NULL;
	
	if (list == NULL)
	{
		list = node;
	}
	else
	{
		pbasic q = list;
		while (q->next != NULL) q = q->next;
		q->next = node;
	}
	return list;
}

pdoubly add_doubly(item data,pdoubly list)
{
	pdoubly node = (pdoubly)malloc(sizeof(doubly));
	node->data = data;
	node->left = NULL;
	node->right = NULL;
	
	if (list==NULL)
	{
		list = node;
	}
	else
	{
		pdoubly q = list;
		while (q->right != NULL) q=q->right;
		q->right = node;
		node->left = q;
	}
	return list;
}

pcircular add_circular(item data,pcircular list)
{
	pcircular p = (pcircular)malloc(sizeof(circular));
	
	p->data = data;
	p->next = NULL;
	
	if (list == NULL)
	{
		list = p;
		tail = p;
		tail->next=list;
	}
	else
	{
		tail->next = p;
		p->next = list;
		tail = p;
	}
	return list;
}


int edit_basic(item data,item value,pbasic list)
{
	if (list==NULL)
	{
		return 0;
	}
	else
	{
		pbasic q = list;
		while (q!=NULL&&data!=q->data) q=q->next;
		if (q==NULL)
		{
			return 0;
		}
		else
		{
			q->data = value;
			return 1;
		}
	}
	return 0;	
}

int edit_doubly(item data,item value,pdoubly list)
{
	if (list==NULL)
	{
		return 0;
	}
	else
	{
		pdoubly q = list;
		while (q!=NULL&&data!=q->data) q=q->right;
		if (q==NULL)
		{
			return 0;
		}
		else
		{
			q->data = value;
			return 1;
		}
	}
	return 0;
}

int edit_circular(item data,item value,pcircular list)
{
	if (list==NULL)
	{
		return 0;
	}
	else
	{
		pcircular q = list;
		while (q!=tail&&data!=q->data) q=q->next;
		if (q==tail&&q->data!=data)
		{
			return 0;
		}
		else
		{
			q->data = value;
			return 1;
		}
	}
	return 0;
}

pbasic remove_basic(item data,pbasic list)
{
	if (list==NULL)
	{
		return NULL;	// rong
	}
	else
	{
		pbasic q = list,tmp;
		while (data!=q->data&&q!=NULL)
		{
			tmp=q;
			q=q->next;
		}
		
		if (list==q)
		{
			list = q->next;
			free(q);
			return list;
		}
		else if (q!=NULL)
		{
			tmp->next = q->next;
			free(q);
			return list;
		}
	}
	return list;
}

pdoubly remove_doubly(item data,pdoubly list)
{
	if (list==NULL)
	{
		return NULL;
	}
	else 
	{
		pdoubly q = list;
		while (data!=q->data&&q!=NULL) q=q->right;
		if (q==NULL)
		{
			return list;
		}
		else if (q==list)
		{
			list = q->right;
			list->left = NULL;
			free(q);
		}
		else if (q->right==NULL)
		{
			q->left->right = NULL;
			free(q);
		}
		else
		{
			q->left->right = q->right;
			q->right->left = q->left;
			free(q);
		}
	}
	return list;
}

pcircular remove_circular(item data,pcircular list)
{
	if (list==NULL)
	{
		return NULL;
	}
	else
	{
		pcircular q = list,tmp;
		while (data!=q->data&&q!=tail)
		{
			tmp=q;
			q=q->next;
		}
		
		if (data!=q->data)
		{
			return list;
		}
		else if(tail==list)
		{
			list = NULL;
			tail = NULL;
			free(list);
		}
		else if (q==list)
		{
			list = q->next;
			tail->next = list;
			free(q);
		}
		else if (q==tail)
		{
			tail = tmp;
			tail->next = list;
			free(q);
		}
		else
		{
			tmp->next = q->next;
			free(q);
		}
	}
	return list;
}

int show_basic(pbasic list)
{
	int len=0;
	
	if (list==NULL)
	{
		return len;
	}
	else
	{
		pbasic q=list;
		while(q!=NULL)
		{
			++len;
			printf("%d ",q->data);
			q=q->next;
		}
		printf("\n");
	}
	return len;
}	
int show_doubly(pdoubly list)
{
	int len = 0;
	if (list==NULL)
	{
		return len;
	}
	else
	{
		pdoubly q=list;
		while(q!=NULL)
		{
			++len;
			printf("%d ",q->data);
			q=q->right;
		}
		printf("\n");
	}
	return len;
}
int show_circular(pcircular list)
{
	int len = 0;
	if (list==NULL)
	{
		return len;
	}
	else
	{
		pcircular q = list;
		while (q!=tail)
		{
			++len;
			printf("%d ",q->data);
			q=q->next;
		}
		if (tail==q)
		{
			++len;
			printf("%d ",tail->data);
		}
		printf("\n");
	}
	return len;
}
