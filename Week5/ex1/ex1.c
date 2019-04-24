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

	// pthread_join(th1, NULL);
	pthread_join(th2, NULL);	// Blocks the calling thread until the thread with identifier equal to the first argument terminates.

	return 0;
}

void *thread_1()
{
	int number;

	srand(time(0));

	while(1)
	{
		pthread_mutex_lock(&lock);

		fclose(f);
		f = fopen("number.txt", "a+");

		printf("- thread %d\n", pthread_self());

		number = rand();
		fprintf(f, "%d ", number);

		fclose(f);

		pthread_mutex_unlock(&lock);

		sleep(2);
	}
	
}

void *thread_2()
{
	FILE *f1, *f2;
	int number;
	char c;

	// f1 = fopen("number.txt", "r");
	// if (f1 == NULL)
	// {
	// 	printf("Thread %d ==> ERROR: open file number.txt\n", pthread_self());
	// 	return ;
	// }

	f2 = fopen("output.txt", "a");

	while(1)
	{
		pthread_mutex_lock(&lock);
		printf("- thread %d\n", pthread_self());

		fscanf(f, "%d", &number);
		number *= number;
		printf("%4d \n", number);
		fprintf(f2, "%d ", number);

		pthread_mutex_unlock(&lock);
		
		sleep(1);
	}
	
	fclose(f1);
	fclose(f2);
}
