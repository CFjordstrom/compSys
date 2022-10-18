#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

#include "job_queue.h"

int job_queue_init(struct job_queue *job_queue, int capacity) {
  if (job_queue == NULL) {
    return 1;
  }
  pthread_mutex_init(job_queue->lock, NULL);
  pthread_cond_init(job_queue->cond, NULL);
  job_queue->capacity = capacity;
  job_queue->dequeue = 0;
  job_queue->enqueue = 0;
  job_queue->n_elms = 0;
  job_queue->destroyed = 0;
  job_queue->array = malloc(sizeof(void*) * capacity);
  return 0;
}

int job_queue_destroy(struct job_queue *job_queue) {
  if (job_queue == NULL) {
    return 1;
  }
  pthread_mutex_lock(job_queue->lock);
  while(job_queue->n_elms > 0) {
    pthread_cond_wait(job_queue->cond, job_queue->lock);
  }
  free(job_queue->array);
  job_queue->destroyed = 1;
  pthread_cond_broadcast(job_queue->cond);
  pthread_mutex_unlock(job_queue->lock);
  return 0;
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  if (job_queue->destroyed) {
    return -1;
  }
  pthread_mutex_lock(job_queue->lock);
  while(job_queue->n_elms == job_queue->capacity) {
    pthread_cond_wait(job_queue->cond, job_queue->lock);
  }
  job_queue->array[job_queue->enqueue % job_queue->capacity] = data;
  job_queue->n_elms++;
  job_queue->enqueue++;
  pthread_cond_broadcast(job_queue->cond);
  pthread_mutex_unlock(job_queue->lock);
  return 0;
}

int job_queue_pop(struct job_queue *job_queue, void **data) {
  if (job_queue->destroyed) {
    return -1;
  }
  pthread_mutex_lock(job_queue->lock);
  while(job_queue->n_elms == 0) {
    pthread_cond_wait(job_queue->cond, job_queue->lock);
  }
  *data = job_queue->array[job_queue->dequeue % job_queue->capacity];
  job_queue->n_elms--;
  job_queue->dequeue++;
  pthread_cond_broadcast(job_queue->cond);
  pthread_mutex_unlock(job_queue->lock);
  return 0;
}
