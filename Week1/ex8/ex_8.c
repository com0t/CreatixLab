#include <stdio.h>
#include <string.h>

int main()
{
	char str[1024];
	int i;
	printf("Nhap xau: ");
	fgets(str,1024,stdin);
	for (i=strlen(str)-1;i>=0;i--)
		printf("%c",str[i]);
	return 0;
}
