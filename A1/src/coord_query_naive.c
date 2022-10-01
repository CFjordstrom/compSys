#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#include "record.h"
#include "coord_query.h"

struct naive_data {
  struct record *rs;
  int n;
};

double euc_dist(double x1, double y1, double x2, double y2) {
  return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

struct naive_data* mk_naive(struct record* rs, int n) {
  struct naive_data* data = malloc(n * sizeof(struct record*) + sizeof(int));
  data->rs = rs;
  data->n = n;
  return data;
}

void free_naive(struct naive_data* data) {
  free(data);
}

const struct record* lookup_naive(struct naive_data *data, double lon, double lat) {
  struct record* closest_record;
  double closest = DBL_MAX;
  
  for(int i = 0; i < data->n; i++) {
    double current = euc_dist(data->rs[i].lon, data->rs[i].lat, lon, lat);
    if (current < closest) {
      closest_record = &data->rs[i];
      closest = current;
    } 
  }
  
  return closest_record;
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}
