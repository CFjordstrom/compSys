#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"

struct naive_data {
  struct record *rs;
  int n;
};

struct naive_data* mk_naive(struct record* rs, int n) {
  struct naive_data* data = malloc(sizeof(struct naive_data));
  data->rs = rs;
  data->n = n;
  return data;
}

void free_naive(struct naive_data* data) {
  free(data);
}

const struct record* lookup_naive(struct naive_data *data, int64_t needle) {
  for(int i = 0; i < data->n; i++) {
    if (data->rs[i].osm_id == needle) {
      return &data->rs[i];
    }
  }
  return NULL;
}

struct index_record {
  int64_t osm_id;
  const struct record* record;
};

struct indexed_data {
  struct index_record* irs;
  int n;
};

struct indexed_data* mk_indexed(struct record* rs, int n) {
  struct indexed_data* indexed_data = malloc(sizeof(struct indexed_data));
  indexed_data->irs = malloc(n * sizeof(struct index_record));
  indexed_data->n = n;
  for (int i = 0; i < n; i++) {
    indexed_data->irs[i].record = &rs[i];
    indexed_data->irs[i].osm_id = rs[i].osm_id;
  }
  return indexed_data;
}

void free_indexed(struct indexed_data* data) {
  free(data->irs);
  free(data);
}

const struct record* lookup_indexed(struct indexed_data *data, int64_t needle) {
  for(int i = 0; i < data->n; i++) {
    if (data->irs[i].osm_id == needle) {
      return data->irs[i].record;
    }
  }
  return NULL;
}

struct sort_data {
  struct record* rs;
  int n;
};

int cmpfunc (const void* a, const void* b) {
  const struct record* r1 = (struct record*)a;
  const struct record* r2 = (struct record*)b;
  if (r1->osm_id > r2->osm_id) {
    return 1;
  }
  else if (r1->osm_id < r2->osm_id) {
    return -1;
  }
  else {
    return 0;
  }
}

const struct record* binary_search(struct record* rs, int64_t needle, int low, int high) {
  while(low <= high) {
    int mid = low + (high-low) / 2;
    if (rs[mid].osm_id == needle) {
      return &rs[mid];
    }
    if (rs[mid].osm_id < needle) {
      low = mid + 1;
    }
    else {
      high = mid - 1;
    }
  }
  return NULL;
}

struct sort_data* mk_sort(struct record* rs, int n) {
  struct sort_data* sort_data = malloc(n * sizeof(struct record*) + sizeof(int));
  sort_data->rs = rs;
  sort_data->n = n;
  qsort(sort_data->rs, n, sizeof(struct record), cmpfunc);
  return sort_data;
}

void free_sort(struct sort_data* data) {
  free(data);
}

const struct record* lookup_sort(struct sort_data *data, int64_t needle) {
  return binary_search(data->rs, needle, 0, data->n);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s FILE\n", argv[0]);
    exit(1);
  }

  int n;
  struct record *rs_n = read_records(argv[1], &n);
  struct record *rs_i = read_records(argv[1], &n);
  struct record *rs_s = read_records(argv[1], &n);
  if ((rs_n && rs_i) && (rs_n && rs_s)) {
    struct naive_data *naive_data = mk_naive(rs_n, n);
    struct indexed_data *indexed_data = mk_indexed(rs_i, n);
    struct sort_data *sort_data = mk_sort(rs_s, n);

    for (int i = 0; i < 100000; i++) {
      int64_t needle = (long)rs_n[rand() % n].osm_id;

      const struct record* r_n = lookup_naive(naive_data, needle);
      const struct record* r_i = lookup_indexed(indexed_data, needle);
      const struct record* r_s = lookup_sort(sort_data, needle);
      assert(r_n->osm_id == r_i->osm_id);
      assert(r_n->osm_id == r_s->osm_id);
    }

    free_naive(naive_data);
    free_indexed(indexed_data);
    free_sort(sort_data);
    free_records(rs_n, n);
    free_records(rs_i, n);
    free_records(rs_s, n);
    return 0;
  } else {
    fprintf(stderr, "Failed to read input from %s (errno: %s)\n",
            argv[1], strerror(errno));
    return 1;
  }
}