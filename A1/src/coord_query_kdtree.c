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

struct node {
  int point;
  int axis;
  struct node* left;
  struct node* right;
};

struct kdtree {
  int axis;
  struct node* node;
};

struct kdtree* mk_kdtree(struct record* rs, int n) {
  
}

void free_kdtree(struct kdtree* data) {
  
}

const struct record* lookup_kdtree(struct kdtree *data, double lon, double lat) {
  
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_kdtree,
                          (free_index_fn)free_kdtree,
                          (lookup_fn)lookup_kdtree);
}
