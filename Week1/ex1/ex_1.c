#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
	char number[3],k;
	short i;
	printf("nhap so co 3 chu so\n");
	printf("> nhap so: ");
	do
	{
		k=1;
		fflush(stdin);
		fgets(number,4,stdin);
		for (i=0; i<3; i++)
		{
			if (number[i] <'0' || number[i] >'9')
			{
				printf("> nhap lai: ");
				break;
			}
			if (i==2) k=0;
		}
	} while(k);
	printf("So nguoc: ");
	for (i=2;i>=0;i--)
		printf("%c",number[i]);
	printf("\n");
}
