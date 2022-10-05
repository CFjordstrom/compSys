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

#define k 2;

struct point {
  double lon;
  double lat;
};

struct node {
  struct point* point;
  int axis;
  struct node* left;
  struct node* right;
};

struct kdtree {
  struct node* root;
};

int cmpfunc_lon(const void* a, const void* b) {
  const struct point* p1 = (struct point*)a;
  const struct point* p2 = (struct point*)b;

  if (p1->lon > p2->lon) {
    return 1;
  }
  else if (p1->lon < p2->lon) {
    return -1;
  }
  else {
    return 0;
  }
}

int cmpfunc_lat(const void* a, const void* b) {
  const struct point* p1 = (struct point*)a;
  const struct point* p2 = (struct point*)b;
  if (p1->lat > p2->lat) {
    return 1;
  }
  else if (p1->lat < p2->lat) {
    return -1;
  }
  else {
    return 0;
  }
}

struct point* mk_points(struct record* rs, int n) {
  struct point* points = malloc(sizeof(struct point) * n);
  for (int i = 0; i < n; i++) {
    points[i].lon = rs[i].lon;
    points[i].lat = rs[i].lat;
  }
  return points;
}

struct point* find_median(struct point* points, int axis, int n) {
  if (axis == 0) {
    qsort(points, n, sizeof(struct point), cmpfunc_lon);
  }
  else {
    qsort(points, n, sizeof(struct point), cmpfunc_lat);
  }
    return &points[n/2+1];
  }

struct node* insert_rec(struct point* points, int depth, int n) {
  int axis = depth % k;
  struct point* median = find_median(points, axis, n);
  struct node* node = malloc(sizeof(struct node));
  node->point = median;
  node->axis = axis;
  if (n == 1) {
    return node;
  }
  node->left = insert_rec(points, depth + 1, n/2);
  node->right = insert_rec(&points[n/2+2], depth + 1, n/2);
  return node;
}

struct kdtree* mk_kdtree(struct record* rs, int n) {
  struct kdtree* kdtree = malloc(sizeof kdtree);
  struct point* points = mk_points(rs, n);
  kdtree->root = insert_rec(points, 0, n);
  return kdtree;
}

void free_kdtree(struct kdtree* data) {
  free(data);
}



const struct record* lookup_kdtree(struct kdtree *data, double lon, double lat) {
  
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_kdtree,
                          (free_index_fn)free_kdtree,
                          (lookup_fn)lookup_kdtree);
}
