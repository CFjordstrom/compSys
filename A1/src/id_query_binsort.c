#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct sort_data {
    struct record* rs;
    int n;
};

int cmpfunc (const void* a, const void* b) {
    const struct record* r1 = (struct record*)a;
    const struct record* r2 = (struct record*)b;
    return (r1->osm_id > r2->osm_id);
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
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_sort,
                    (free_index_fn)free_sort,
                    (lookup_fn)lookup_sort);
}