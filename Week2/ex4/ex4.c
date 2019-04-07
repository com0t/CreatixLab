#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	int n,i,num;
	
	printf("Nhap so thuoc 2<=n<5\n");
	do
	{
		printf("Nhap so: ");
		scanf("%d",&n);
	} while (n<2||n>=5);
	
	i=1;
	
	while (len((num=i*11))<=n)
	{
		printf("%d\n",num);
		++i;
	}
		
}

int len(int n)
{
	char buf[5];
	itoa(n,buf,10);
	return strlen(buf);
}
