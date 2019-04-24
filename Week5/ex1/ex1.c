#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

void * thread_1();		// Generate random number and write into number.txt
void * thread_2();		// Read file number.txt and square, write number to ouput.txt

pthread_mutex_t lock;

int main()
{
	pthread_t th1,th2;

	pthread_create(
		&th1, 			// Pointer to thread
		NULL, 			// Specifies attributes, use default attributes
		thread_1, 		// Function to execute thread
		NULL 			// Argument is used to pass arguments to the function
		);

	pthread_create(&th2, NULL, thread_2, NULL);

	pthread_join(th1, NULL);
	pthread_join(th2, NULL);	// Blocks the calling thread until the thread with identifier equal to the first argument terminates.

	return 0;
}

void *thread_1()
{
	int number;
	FILE *f;

	srand(time(0));

	while(1)
	{
		pthread_mutex_lock(&lock);

		f = fopen("number.txt", "a");

		number = rand() % 1000;
		fprintf(f, "%d ", number);
		// printf("Write into number.txt: %d\n", number);

		fclose(f);
		pthread_mutex_unlock(&lock);

		sleep(2);
	}
	
}

void *thread_2()
{
	FILE *f1, *f2;
	int number, i;
	char c = 0;
	char buff[12];
	short check = 0;

	f2 = fopen("output.txt", "a");
	f1 = fopen("number.txt", "r");
	i = 0;

	while(1)
	{
		pthread_mutex_lock(&lock);
		c = fgetc(f1);
		// printf("point: %d\n", ftell(f1));
		if (c != EOF && c != ' ')
		{
			check = 0;
			buff[i++] = c;
		}
		else if ((c == ' ' || c == EOF) && check == 0 && i != 0)
		{
			buff[i] = 0;
			number = atoi(buff);
			number *= number;
			printf("%d\n", number);
			fprintf(f2, "%d ", number);
			i = 0;
			check = 1;
		}
		pthread_mutex_unlock(&lock);
		
		sleep(1);
	}
	
	fclose(f1);
	fclose(f2);
}
