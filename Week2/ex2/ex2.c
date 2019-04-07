#include <stdio.h>
#include <stdlib.h>

void bubbleSort(int *,int);
void insertSort(int *,int);
void selectionSort(int *,int);
void show(int *a,int);
void swap(int *,int *);


int main()
{
	int i,n;
	int a[150],b[50],c[50];
	printf("Nhap so: ");
	scanf("%d",&n);
	for (i=0;i<n;++i)
	{
		scanf("%d",&a[i]);
		b[i]=c[i]=a[i];
	}
	
	printf("\n---------BubleSort----------\n");
	bubbleSort(a,n);
	show(a,n);
	
	printf("\n---------InsertSort----------\n");
	insertSort(b,n);
	show(b,n);
	
	printf("\n---------SelectionSort----------\n");
	selectionSort(c,n);
	show(c,n);
}

void bubbleSort(int *a,int n)
{
	int i,j;
	for (i=0;i<n-1;++i)
	{
		for (j=n-1;j>i;--j)
		{
			if (a[j]<a[j-1])
			{
				swap(&a[j],&a[j-1]);
			}
		}
	}
}

void insertSort(int *a,int n)
{
	int i,j;
	for (i=0;i<n;++i)
	{
		for (j=i;j>0;--j)
		{
			if (a[j]<a[j-1])
			{
				swap(&a[j],&a[j-1]);
			}
		}
	}
}

void selectionSort(int *a,int n)
{
	int i,j,min;
	for (i=0;i<n-1;++i)
	{
		min = i;
		for (j=i+1;j<n;j++)
		{
			if (a[min]>a[j]) min = j;
		}
		
		if (min!=i) swap(&a[i],&a[min]);
	}
}

void show(int *a,int n)
{
	int i;
	for (i=0;i<n;++i)
	{
		printf("%3d",a[i]);
	}
	printf("\n");
}

void swap(int *a,int *b)
{
	int tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;	
}
