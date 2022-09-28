#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct index_record {
  int64_t osm_id;
  const struct record* record;
};

struct indexed_data {
  struct index_record* irs;
  int n;
};

struct indexed_data* mk_indexed(struct record* rs, int n) {
  struct indexed_data* indexed_data = malloc(n * sizeof(struct index_record*) + sizeof(int));
  indexed_data->n = n;
  for (int i = 0; i < n; i++) {
    struct index_record* ir = malloc(sizeof(struct index_record*));
    ir->osm_id = rs[i].osm_id;
    ir->record = &rs[i];
    indexed_data->irs[i] = ir;
  }
  return indexed_data;
}

void free_indexed(struct indexed_data* data) {
  for(int i = 0; i < data->n; i++) {
    free(&data->irs[i]);
  }
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
 
int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_indexed,
                    (free_index_fn)free_indexed,
                    (lookup_fn)lookup_indexed);
}