#include <stdio.h>
#include <stdlib.h>
 
int USCLN(int a, int b) {
    if (b == 0) return a;
    return USCLN(b, a % b);
}

int BSCNN(int a, int b) {
    return (a * b) / USCLN(a, b);
}
 
int main() {
    int a, b;
    printf("Nhap so nguyen duong a = ");
    scanf("%d", &a);
    printf("Nhap so nguyen duong b = ");
    scanf("%d", &b);
	if (a==0 || b==0)
	{
		printf("co so 0");
		exit(0);
	}
    // tinh USCLN cua a và b
    printf("USCLN cua %d va %d la: %d\n", a, b, USCLN(a, b));
    // tinh BSCNN cua a và b
    printf("BSCMM cua %d va %d la: %d", a, b, BSCNN(a, b));
}

