#include <stdio.h>
#include <string.h>
 
void chuanhoachuoi(char[]);
void strdelete(char *s,int vt,int sl);
void straddspace(char *s, int vt,int sl);

int main()
{
   char s[1024];
   
   printf("Nhap vao chuoi:");
   fgets(s,1024,stdin);
   chuanhoachuoi(s);
   printf("Chuoi sau chuan hoa:%s",s);
}
 
void strdelete(char *s,int vt,int sl)
{  
    int n=strlen(s),i;
    for (i=vt;i<=n-sl;++i)
        s[i]=s[i+sl];
}

void straddspace(char *s, int vt,int sl)
{
	int n=strlen(s),i;
    for (i=n;i>vt;--i)
        s[i]=s[i-sl];
	s[vt+1] = ' ';
}
void chuanhoachuoi(char a[])
{
    int i;
    while(a[strlen(a)-1]==' ')// xoa het cac ki tu trong o cuoi chuoi
        a[strlen(a)-1]=0;
    while(a[0]==' ')          //xoa het cac ki tu trong o dau chuoi
        strdelete(a,0,1);
    for(i=0;i<strlen(a);++i)
        if((a[i]>=65)&&(a[i]<=90))  //kiem tra co phai la ky tu hoa
            a[i]+=32;        //chuyen tat ca ve ky tu thuong
 
    i=0;
    while(i<strlen(a))                //xoa hai ki tu lien tiep
	    if((a[i]=='.')&&(a[i+1]=='.') || (a[i]==',')&&(a[i+1]==',') || (a[i]==' ')&&(a[i+1]==' ') 
			|| (a[i]==' ')&&(a[i+1]=='.') || (a[i]==' ')&&(a[i+1]==','))
	        strdelete(a,i,1);
	    else ++i;
	for (i=0;i<strlen(a);i++)
	{
		if (a[i]=='.'||a[i]==',')
			straddspace(a,i,1);
	}
	while(a[strlen(a)-1]==' ')// xoa het cac ki tu trong o cuoi chuoi
        a[strlen(a)-1]=0;
   a[0]=toupper(a[0]);//chuyen chu cai dau tien thanh chu hoa
}
