#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_THREAD 4

typedef struct blocked
{
	pthread_t tid;
	struct blocked *next;
} Blocked,*pBlocked;

typedef struct semaphore
{
	int value;
	pBlocked list;
} sema,*psema;

void *producer(void *arg);
void *consumer(void *arg);
void wait(sema *s);
void post(sema *s);
void cleanup_handler(void *arg);
void init();

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond[MAX_THREAD];
pthread_t th[MAX_THREAD];
//pBlocked list = NULL;
sema empty,full,mutex;
int counter;

int main()
{
	init();
	pthread_create(&th[0],NULL,consumer,NULL);
	pthread_create(&th[1],NULL,producer,NULL);
	pthread_create(&th[2],NULL,consumer,NULL);
	pthread_create(&th[3],NULL,producer,NULL);
	pthread_join(th[0],NULL);
	pthread_join(th[1],NULL);
	pthread_join(th[2],NULL);
	pthread_join(th[3],NULL);
	return 0;
}

void init()
{
	int i;
	counter = 0;
	mutex.value = 1;
	mutex.list = NULL;
	empty.value = 5;
	empty.list = NULL;
	full.value = 0;
	full.list = NULL;
	for (i = 0;i<MAX_THREAD;i++) {
		cond[i] = PTHREAD_MUTEX_INITIALIZER;
	}
}

void cleanup_handler(void *arg)
{
	printf("Thread %d calling cleanup_handler\n",pthread_self());
}

void *producer(void *arg)
{
	while(1) {
		wait(&empty);
		wait(&mutex);
		printf("----> thread %d produce: %d\n",pthread_self(),++counter);
		post(&mutex);
		post(&full);
	}
}

void *consumer(void *arg)
{
		while(1) {
			wait(&full);
			wait(&mutex);
			printf("----> thread %d consume: %d\n",pthread_self(),--counter);
			post(&mutex);
			post(&empty);
		}	
}

void wait(sema *s)
{
	int result = 0;
	pthread_testcancel();
	if ((result = pthread_mutex_lock(&lock)) == 0) {
//		printf("Function P Thread %d Start\n",pthread_self());
//		pthread_cleanup_push(cleanup_handler,NULL);
		--(s->value);
		if (s->value < 0) {
			pBlocked p = (pBlocked)malloc(sizeof(struct blocked)),q = s->list;
			p->tid = pthread_self();
			p->next = NULL;
			if (s->list == NULL) {
				s->list = p;
			} else {
				while(q->next != NULL) q = q->next;
				q->next = p;
			}
//			printf("--> Add thread %d to list\n",p->tid);
			result = pthread_cond_wait(&cond[p->tid - 1],&lock);
//			pthread_cleanup_pop(result);
		}
//		printf("Thread %d over P\n",pthread_self());
		pthread_mutex_unlock(&lock);
	}
}

void post(sema *s)
{
	int result = 0;
	if ((result = pthread_mutex_lock(&lock)) == 0) {
//		printf("Function V Thread %d Start\n",pthread_self());
		++(s->value);
		if (s->value <= 0) {
			if (s->list!= NULL) {
				pBlocked q = s->list;
				s->list = s->list->next;
				
				pthread_cond_signal(&cond[q->tid -1]);
//				printf("Unlock thread %d from %d\n",q->tid,pthread_self());
			}
		}
//		printf("Thread %d over V\n",pthread_self());
		pthread_mutex_unlock(&lock);
	}
}