#include <stdio.h>
#include <string.h>
 
void xoachuoi(char[]);
void strdelete(char *s,int vt,int sl);

int main()
{
   char s[1024];
   
   printf("Nhap vao chuoi:");
   fgets(s,1024,stdin);
   xoachuoi(s);
   printf("Chuoi: %s",s);
}
 
void strdelete(char *s,int vt,int sl)
{  
    int n=strlen(s),i;
    for (i=vt;i<=n-sl;++i)
        s[i]=s[i+sl];
    s[i]=0;
}

void xoachuoi(char a[])
{
    int i;
    i=0;
    while(i<strlen(a))        
	    if(a[i]=='s'&&a[i+1]=='d'&&a[i+2]=='f')
	    {
	        strdelete(a,i,3);
	        i=0;
		}
	    else ++i;
}
