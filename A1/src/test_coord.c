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
#include "record.c"
#include "coord_query.h"

#define k 2

struct node {
  int axis;
  struct node* left;
  struct node* right;
  struct record* r;
  double point[k];
};

struct kdtree {
  struct node* root;
};

double euc_dist(double x1, double y1, double x2, double y2) {
  return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

int cmpfunc_lon(const void* a, const void* b) {
  const struct record* p1 = (struct record*)a;
  const struct record* p2 = (struct record*)b;

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
  const struct record* p1 = (struct record*)a;
  const struct record* p2 = (struct record*)b;
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

struct record* find_median(struct record* rs, int axis, int n) {
  if (axis == 0) {
    qsort(rs, n, sizeof(struct record), cmpfunc_lon);
  }
  else {
    qsort(rs, n, sizeof(struct record), cmpfunc_lat);
  }
    return &rs[n/2];
  }

struct node* insert_rec(struct record* rs, int depth, int n) {
  if (n == 0) {
    return NULL;
  }
  int axis = depth % k;
  struct record* median = find_median(rs, axis, n);
  struct node* node = malloc(sizeof(struct node));
  node->point[0] = median->lon;
  node->point[1] = median->lat;
  node->axis = axis;
  node->r = median;
  node->left = insert_rec(rs, depth+1, n/2);
  node->right = insert_rec(&rs[n/2+1], depth+1, n-n/2-1);
  return node;
}

struct kdtree* mk_kdtree(struct record* rs, int n) {
  struct kdtree* kdtree = malloc(sizeof kdtree);
  kdtree->root = insert_rec(rs, 0, n);
  return kdtree;
}

void free_rec(struct node* node) {
  if (node == NULL) {
    return;
  }
  free_rec(node->left);
  free_rec(node->right);
  free(node);
}

void free_kdtree(struct kdtree* data) {
  free_rec(data->root);
  free(data);
}

void lookup_rec(struct node* closest, struct record* query, struct node* node) {
  if (node == NULL) {
    return;
  }
  double current_dist = euc_dist(node->point[0], node->point[1], query->lon, query->lat);
  double closest_dist = euc_dist(closest->point[0], closest->point[1], query->lon, query->lat);
  printf("current dist: %f, closest dist: %f\n", current_dist, closest_dist);
  
  if  (current_dist < closest_dist) {
    closest->r = node->r;
    closest->point[0] = node->point [0];
    closest->point[1] = node->point[1];
  }
  double diff;
  if (node->axis == 0) {
    diff = node->point[node->axis] - query->lon;
  } else {
    diff = node->point[node->axis] - query->lat;
  }
  double radius = euc_dist(query->lon, query->lat, closest->point[0], closest->point[1]);
  if ((diff >= 0) | (radius > fabs(diff))) {
    lookup_rec(closest, query, node->left);
  }
  if ((diff >= 0) | (radius > fabs(diff))) {
    lookup_rec(closest, query, node->right);
  }
}

const struct record* lookup_kdtree(struct kdtree *data, double lon, double lat) {
  struct record* query = malloc(sizeof(struct record));
  query->lon = lon;
  query->lat = lat;
  struct node* closest = malloc(sizeof(struct node));
  closest->point[0] = DBL_MAX;
  closest->point[1] = DBL_MAX;
  lookup_rec(closest, query, data->root);
  printf("lon: %f, lat: %f\n", closest->point[0], closest->point[1]);
  return closest->r;
}

struct naive_data {
  struct record *rs;
  int n;
};

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
  if (argc != 2) {
    fprintf(stderr, "Usage: %s FILE\n", argv[0]);
    exit(1);
  }

  int n;
  struct record *rs_n = read_records(argv[1], &n);
  struct record *rs_kd = read_records(argv[1], &n);
  if (rs_n && rs_kd) {
    struct naive_data *naive_data = mk_naive(rs_n, n);
    struct kdtree *kdtree = mk_kdtree(rs_kd, n);

    for (int i = 0; i < 100000; i++) {
      double lon = rand() % (180 + 1);
      double lat = rand() % (90 + 1);

      const struct record* r_n = lookup_naive(naive_data, lon, lat);
      const struct record* r_kd = lookup_kdtree(kdtree, lon, lat);
      assert(rs_n->osm_id == rs_kd->osm_id);
    }

    free_naive(naive_data);
    free_kdtree(kdtree);
    free_records(rs_n, n);
    free_records(rs_kd, n);
    return 0;
  } else {
    fprintf(stderr, "Failed to read input from %s (errno: %s)\n",
            argv[1], strerror(errno));
    return 1;
  }
}
