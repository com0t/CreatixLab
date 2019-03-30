#include <stdio.h>

int main()
{
	int n;
	int i,j;
	printf("Nhap do cao h: ");
	scanf("%d",&n);
	printf("\n");
	for (i=0;i<n;i++)
	{
		for (j=0;j<n-i;j++) printf("  ");
		for (j=2*i+1;j>0;j--) printf("* ");
		printf("\n");
	}
}
