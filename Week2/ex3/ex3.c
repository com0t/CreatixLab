/*
* Su dung thuat toan tim duong di ngan nhat Dijkstra de tim duong di thich hop
* Y tuong:
* 	+ Doc file va chuyen doi cap gia tri x,y sang to do trong ma tran
*	+ Su dung thuat toan Dijkstra de tim qua duong di
*/

#include <stdio.h>
#include <stdlib.h>

#define INFINITY 9999
#define MAX 100

void dijkstra(int G[MAX][MAX], int n, int startnode,int endnode)
{

	int cost[MAX][MAX], distance[MAX], pred[MAX];
	int visited[MAX], count, mindistance, nextnode, i, j;
	int tmp[MAX],k;

	//pred[] luu cac dinh ma khoang cach ngan nhat tu no den dinh nguon
	//count dem so nut ma di qua den den dich
	//Tao ma tran trong so
	for (i = 0; i<n; i++)
		for (j = 0; j<n; j++)
			if (G[i][j] == 0)
				cost[i][j] = INFINITY;
			else
				cost[i][j] = G[i][j];

	//Khoi tao mang pred[],distance[] and visited[]
	for (i = 0; i<n; i++)
	{
		distance[i] = cost[startnode][i];
		pred[i] = startnode;
		visited[i] = 0;
	}

	distance[startnode] = 0;
	visited[startnode] = 1;
	count = 1;

	while (count<n - 1)
	{
		mindistance = INFINITY;

		//Nut ke tiep co khoang cach nho nhat
		for (i = 0; i<n; i++)
			if (distance[i]<mindistance && !visited[i])
			{
				mindistance = distance[i];
				nextnode = i;
			}

		//Kiem tra neu co duong dan tot hon: tinh lai khoang cach cac nut chua duoc tham      
		visited[nextnode] = 1;
		for (i = 0; i<n; i++)
			if (!visited[i])
				if (mindistance + cost[nextnode][i]<distance[i])
				{
					distance[i] = mindistance + cost[nextnode][i];
					pred[i] = nextnode;
				}
		count++;
	}

	//In ra duong di va khoang cach den moi nut
	for (i = 0; i<n; i++)
		if (i != startnode && endnode==i)
		{
			tmp[k++]=i;

			j = i;
			do
			{
				j = pred[j];
				tmp[k++] = j;
			} while (j != startnode);
		}
	
	// In ra theo thu tu yeu cau cua de bai
	printf("\n");
	for (i=k-1;i>0;--i)
		printf("%4d",tmp[i]);
	printf("\n");
}

int main()
{
	FILE *f;
	int n,m,v,u;
	int a[100][100];
	int x,y,i,j;
	
	f = fopen("input.txt","r");
	if (f==NULL)
	{
		perror("Error file\n");
		exit(1);
	}
	
	// Doc file gan vao do thi	
	fscanf(f,"%d%d%d%d",&n,&m,&v,&u);
	for (i=0;i<m;++i)
	{
		fscanf(f,"%d%d",&x,&y);
		a[x][y] = 1;
	}
	fclose(f);	
	
	
	// Xuat ma do thi
//	printf("    ");
//	for (i=0;i<=m;++i)
//		printf("%4d",i);
//	printf("\n");
//	for (i=0;i<=m;++i)
//	{
//		for (j=0;j<=m;++j)
//		{
//			if (j==0) printf("%4d",i);
//			printf("%4d",a[i][j]);
//		}
//		printf("\n");
//	}
	
	
	// Thuat toan tim duong
	dijkstra(a,m+1,v,u);
}
