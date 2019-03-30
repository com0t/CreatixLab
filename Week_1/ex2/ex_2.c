#include <stdio.h>
#include <string.h>

int main()
{
	char str[256];
	short hoa=0,thuong=0,i;
	printf("Nhap xau: ");
	fgets(str,256,stdin);
	for (i=0;i<strlen(str);i++)
	{
		if (str[i]>='A' && str[i]<='Z')
			++hoa;
		if (str[i]>='a' && str[i]<='z')
			++thuong;					
	}
	printf("Ky tu hoa: %d\n",hoa);
	printf("Ky tu thuong: %d\n",thuong);
}
