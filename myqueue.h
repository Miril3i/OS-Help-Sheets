#ifndef MYQUEUE_H_
#define MYQUEUE_H_

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/queue.h> // see queue(7) & stailq(3)
#include <pthread.h> // for pthread_mutex_t

struct myqueue_entry {
	int value;
	STAILQ_ENTRY(myqueue_entry) entries;
};

STAILQ_HEAD(myqueue_head, myqueue_entry);

typedef struct myqueue_head myqueue;

typedef struct {
	myqueue queue;
	pthread_mutex_t lock;
} myqueue_safe;

static void myqueue_init(myqueue_safe* q) {
	STAILQ_INIT(&q->queue);
	pthread_mutex_init(&q->lock, NULL);
}

static bool myqueue_is_empty(myqueue_safe* q) {
	pthread_mutex_lock(&q->lock);
	bool is_empty = STAILQ_EMPTY(&q->queue);
	pthread_mutex_unlock(&q->lock);
	return is_empty;
}

static void myqueue_push(myqueue_safe* q, int value) {
	struct myqueue_entry* entry = malloc(sizeof(struct myqueue_entry));
	entry->value = value;
	pthread_mutex_lock(&q->lock);
	STAILQ_INSERT_TAIL(&q->queue, entry, entries);
	pthread_mutex_unlock(&q->lock);
}

static int myqueue_pop(myqueue_safe* q) {
	pthread_mutex_lock(&q->lock);
	assert(!STAILQ_EMPTY(&q->queue));
	struct myqueue_entry* entry = STAILQ_FIRST(&q->queue);
	const int value = entry->value;
	STAILQ_REMOVE_HEAD(&q->queue, entries);
	free(entry);
	pthread_mutex_unlock(&q->lock);
	return value;
}

#endif
