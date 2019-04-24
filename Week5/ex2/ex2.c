#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t mutex;
int counter;

void* thread(void* arg)
{
	// Wait
	sem_wait(&mutex);

	while (counter <= 120)
	{
		printf("- Thread %d: %d\n", pthread_self(), counter);
		++counter;
	}

	// Signal
	sem_post(&mutex);
}

int main()
{
	pthread_t th1,th2,th3,th4,th5,th6;
	
	counter = 0;
	sem_init(&mutex, 0, 4);

	// Begin time
	clock_t begin = clock();
	
	pthread_create(&th1, NULL, thread, NULL);
	pthread_create(&th2, NULL, thread, NULL);
	pthread_create(&th3, NULL, thread, NULL);
	pthread_create(&th4, NULL, thread, NULL);
	pthread_create(&th5, NULL, thread, NULL);
	pthread_create(&th6, NULL, thread, NULL);
	
	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	pthread_join(th3, NULL);
	pthread_join(th4, NULL);
	pthread_join(th5, NULL);
	pthread_join(th6, NULL);
	
	// End time
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	
	printf("--> CPU time: %ld ms\n", time_spent);

	printf("________ Main Exit _________\n");

	return 0;
}
