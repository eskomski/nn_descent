#include "knnd.h"
#include "vec.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>

int make_test_data(dataset_t* data, int n, int d)
{
    data->values = malloc((sizeof(float*) * n) + (n * d * sizeof(float)));
    if (!(data->values)) return 1;

    for (int i = 0; i < n; i++) {
        data->values[i] = (float*)(data->values + n) + i * d;
        for (int j = 0; j < d; j++) {
            data->values[i][j] = (float)rand() / 100000000.f;
        }
    }

    data->size = n;
    data->dim = d;
    return 0;
}

vec_t* heap_list_create(int size, int k)
{
    vec_t* hl = malloc(sizeof(vec_t) * size);
    if (!hl) return NULL;

    for (int i = 0; i < size; i++) {
        if (!vec_create(&hl[i], k)) return NULL;
    }

    return hl;
}

void heap_list_free(vec_t* hl, int size)
{
    for (int i = 0; i < size; i++) {
        vec_free(&hl[i]);
    }

    free(hl);
}

void max_heapify(vec_t* h, int i)
{
    int l = (i*2)+1, r = (i*2)+2, max;
    max = (l < h->_capacity && h->arr[l].val > h->arr[i].val) ? l : i;
    max = (r < h->_capacity && h->arr[r].val > h->arr[max].val) ? r : max;
    if (max != i) {
        node_t t = h->arr[i];
        h->arr[i] = h->arr[max];
        h->arr[max] = t;
        max_heapify(h, max);
    }
}

int nn_update(vec_t* h, node_t* node)
{
    if (node->val >= h->arr[0].val || heap_find_by_index(h, node->id) != -1) return 0;

    h->arr[0] = *node;
    max_heapify(h, 0);
    return 1;
}

vec_t* nn_descent(dataset_t data, float(*metric)(float*, float*, int), int k, float rho, float delta)
{
    if (k >= data.size) {
        printf("error: neighborhood size must be less than dataset size\n");
        return NULL;
    }
    printf("beginning nn descent...\n");
    // TODO: really should check if any of these turn out to fail,
    // then free the ones that didn't :S

    vec_t* B     = heap_list_create(data.size, k);
    vec_t* old   = heap_list_create(data.size, k);
    vec_t* old_r = heap_list_create(data.size, k);
    vec_t* new   = heap_list_create(data.size, k);
    vec_t* new_r = heap_list_create(data.size, k);

    // initialize heap list B
    for (int i = 0; i < data.size; i++) {
        // sample 
        for (int j = 0; j < k;) {
            node_t t = {(int)rand() % data.size, FLT_MAX, true};
            if (heap_insert(&B[i], &t) == 1) j++;
        }
    }

    int c;
    int pk = k * rho;
    int stop_iter = delta * data.size * k;
    vec_t sample_buf;
    vec_create(&sample_buf, pk);
    do {
        vec_clear(old);
        vec_clear(new);
        for (int v = 0; v < data.size; v++) {   // TODO: parallel
            for (int j = 0; j < B[v].size; j++) {
                if (!(B[v].arr[j].new) && B[v].arr[j].id != v) {
                    heap_insert(&old[v], &B[v].arr[j]);
                }
            }
            sample_neighbors(&new[v], &B[v], pk, true);
        }

        reverse_heap_list(old_r, old, data.size);
        reverse_heap_list(new_r, new, data.size);

        c = 0;
        for (int v = 0; v < data.size; v++) {   // TODO: parallel
            sample_neighbors(&sample_buf, &old_r[v], pk, false);
            if (!heap_union(&old[v], &sample_buf)) return NULL;

            sample_neighbors(&sample_buf, &new_r[v], pk, false);
            if (!heap_union(&new[v], &sample_buf)) return NULL;
            for (int i = 0; i < new[v].size; i++) {
                for (int j = 0; j < new[v].size; j++) {
                    if (i == j) continue;

                    node_t u1, u2;
                    u1.id = new[v].arr[i].id;
                    u2.id = new[v].arr[j].id;

                    if (u1.id <= u2.id || heap_find_by_index(&old[v], u2.id) == -1) continue;
                    float l = metric(data.values[u1.id], data.values[u2.id], data.dim);
                    u1.val = u2.val = l;
                    c += nn_update(&B[u1.id], &u2);
                    c += nn_update(&B[u2.id], &u1);
                }
            }
        }
        printf("iteration complete: %d / %d\n", c, stop_iter);
    } while (c >= stop_iter);

    printf("done, cleaning up...\n");

    heap_list_free(old, data.size);
    heap_list_free(old_r, data.size);
    heap_list_free(new, data.size);
    heap_list_free(new_r, data.size);

    return B;
}

int reverse_heap_list(vec_t* dst, vec_t* src, int size)
{
    vec_clear(dst);
    for (int v = 0; v < size; v++) {
        for (int u = 0; u < src[v].size; u++) {
            node_t t = src[v].arr[u];
            int dst_index = t.id;
            t.id = v;
            if (heap_insert(&dst[dst_index], &t) == -1) return 1;
        }
    }

    return 0;
}

int sample_neighbors(vec_t* dst, vec_t* src, int k, bool use_new)
{
    if (src->size == 0) return 0;

    vec_clear(dst);

    int j = 0;
    for (int i = 0; i < src->size && j < k; i++) {
        int rand_index = rand() % src->size;
        node_t t = src->arr[rand_index];
        if (use_new && !t.new) continue;
        if (use_new) {
            src->arr[rand_index].new = false;
            t.new = false;
        }
        if (heap_insert(dst, &t)) j++;
    }

    /*
    for (int i = 0; i < src->size && j < k; i++) {
        if (use_new && !src->arr[i].new) continue;
        if (heap_insert(dst, &(src->arr[i])) == -1) return -1;
        if (use_new) src->arr[i].new = false;
        j++;
    }
    */

    return j;
}

