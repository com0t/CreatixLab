#include <stdio.h>

int main()
{
	int ngay,thang,nam;
	short k;
	do
	{
		k=1;
		printf("Ngay: ");
		scanf("%d",&ngay);
		printf("Thang: ");
		scanf("%d",&thang);
		printf("Nam: ");
		scanf("%d",&nam);
		if ((thang>=1 && thang<=12) && (ngay>=1 && ngay<=31))
		{
			if (((nam%4==0) && (nam%100!=0)) || (nam%400)==0)
			{
				if (thang==2 && ngay>29)
				{
					printf("Khong hop le\n");
					continue;
				}
				else if (thang==4||thang==6||thang==9||thang==11 && ngay>30)
				{
					printf("Khong hop le\n");
					continue;
				}
				printf("Nam nhuan\n");
				k=0;
			}
			else
			{
				if (thang==2 && ngay>28)
				{
					printf("Khong hop le\n");
					continue;
				}
				else if (thang==4||thang==6||thang==9||thang==11 && ngay>30)
				{
					printf("Khong hop le\n");
					continue;
				}
				printf("Nam khong nhuan\n");
				k=0;
			}
		}
		else
		{
			printf("Khong hop le\n");
		}
	} while(k);
}
