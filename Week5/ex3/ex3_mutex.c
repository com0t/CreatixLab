#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

int mutex = 0;
int counter = 0;

void * thread(void * arg)
{
	// Wait until mutex = 0
	while(mutex);

	// Excute
	while (counter <= 120)
	{
		// lock mutex = 1
		mutex = 1;

		printf("- Thread %d: %d\n", pthread_self(), counter);
		++counter;
		
		// unlock mutex = 1;
		mutex = 0;
		sleep(1);
	}
}

int main()
{
	pthread_t th1, th2, th3;

	pthread_create(&th1, NULL, thread, NULL);
	pthread_create(&th2, NULL, thread, NULL);
	pthread_create(&th3, NULL, thread, NULL);

	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	pthread_join(th3, NULL);
	
	printf("END\n");
	return 0;
}

void lock()
{
	
}
