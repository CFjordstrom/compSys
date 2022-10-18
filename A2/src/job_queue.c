#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"

int job_queue_init(struct job_queue *job_queue, int capacity) {
  if (job_queue == NULL) {
    return 1;
  }
  pthread_mutex_init(&job_queue->lock, NULL);
  pthread_cond_init(&job_queue->cond, NULL);
  job_queue->capacity = capacity;
  job_queue->front = 0;
  job_queue->back = 0;
  job_queue->n_elms = 0;
  job_queue->destroyed = 0;
  job_queue->array = malloc(sizeof(void*) * capacity);
  return 0;
}

int job_queue_destroy(struct job_queue *job_queue) {
  if (job_queue == NULL) {
    return 1;
  }
  
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  assert(0);
}

int job_queue_pop(struct job_queue *job_queue, void **data) {
  assert(0);
}
