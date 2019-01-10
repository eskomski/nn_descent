#include "knnd.h"
#include "vec.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>

// this won't be useful for assessing the correctness of this, but
// at least we'll see if it runs w/o crashing
int make_test_data(dataset_t* data, int n, int d)
{
    data->values = malloc((sizeof(float*) * n) + (n * d * sizeof(float)));
    if (!(data->values)) return 1;

    for (int i = 0; i < n; i++) {
        data->values[i] = (float*)(data->values + n) + i * d;
        for (int j = 0; j < d; j++) {
            data->values[i][j] = (float)rand() / 100000.f;
        }
    }

    data->size = n;
    data->dim = d;
    return 0;
}

vec_t* nn_create_heap_list(int size, int k)
{
    vec_t* hl = malloc(sizeof(vec_t) * size);
    if (!hl) {
        printf("error: heap list failed to allocate\n!");
        return NULL;
    }

    for (int i = 0; i < size; i++) {
        if (vec_create(&hl[i], k)) {
            printf("error: heap list failed to allocate\n!"); // TODO: different msg
            return NULL;
        }
    }

    return hl;
}

void nn_free_heap_list(vec_t* hl, int size)
{
    for (int i = 0; i < size; i++) {
        vec_free(&hl[i]);
    }

    //free(hl);
}

int nn_update(vec_t* h, node_t* node)
{
    if (node->val >= h->arr[0].val || heap_find_by_index(h, node->id) != -1) return 0;

    //if (_vec_check_size(h)) return -1;

    h->arr[0] = *node;
    //int l, r, p, i = 0;
    //node_t t;

    int l, r, max, n;
    for (int i = (h->_capacity)/2 - 1; i > 0; i--) {
        node_t t;
        n = 0;
        while (n < h->_capacity) {
            l = n*2 + 1;
            r = n*2 + 2;

            if (l > h->_capacity) break;
            max = (h->arr[l].val >= h->arr[n].val ? l : n);
            printf("n=%d, l=%d, r=%d, min=%d\n", n, l, r, max);
            if (r < h->_capacity && h->arr[r].val > h->arr[max].val) max = r;
            if (n != max) {
                t = h->arr[n];
                h->arr[n] = h->arr[max];
                h->arr[max] = t;
                n = max;
            } else {
                break;
            }
        }
    }
    /*
    while (p >= 0 && h->arr[p].val < h->arr[n].val) {
        t = h->arr[p];
        h->arr[p] = h->arr[n];
        h->arr[n] = t;
        n = p;
        p = (n-1)/2;
    }
    */

    //h->size++;
    
    return 1;
}

// note that caller must free heap list B
vec_t* nn_descent(dataset_t data, float(*metric)(float*, float*, int), int k, float rho, float delta)
{
    printf("beginning nn descent...\n");
    // TODO: really should check if any of these turn out to fail,
    // then free the ones that didn't :S

    vec_t* B, * old, * old_r, * new, * new_r;
    B = nn_create_heap_list(data.size, k);
    old = nn_create_heap_list(data.size, k);
    old_r = nn_create_heap_list(data.size, k);
    new = nn_create_heap_list(data.size, k);
    new_r = nn_create_heap_list(data.size, k);

    // initialize heap list B
    node_t t;
    for (int i = 0; i < data.size; i++) {
        for (int j = 0; j < k; j++) {
            t.id = (int)rand() % data.size; // TODO: this will sample with replacement, which we might not want?
            t.val = FLT_MAX;
            t.new = true;
            vec_insert(&B[i], &t);
        }
    }

    int c = 0, rand_index;
    node_t u1, u2;
    int pk = k * rho;
    int stop_iter = delta * data.size * k;
    float l;
    vec_t sample_buf;
    vec_create(&sample_buf, pk);
    while (c < stop_iter) {
        // TODO: parallel
        for (int v = 0; v < data.size; v++) {
            for (int j = 0; j < B[v].size; j++) {
                if (!(B[v].arr[j].new) && B[v].arr[j].id != v) {
                    heap_insert(&old[v], &(B[v].arr[j]));
                }
            }
            sample_neighbors(&new[v], &B[v], pk, true);
        } // both old and new are heapsorted at this point
        puts("reversal");
        reverse_heap_list(old_r, old, data.size);
        reverse_heap_list(new_r, new, data.size);
        // TODO: parallel
        for (int v = 0; v < data.size; v++) {
            printf("union %u\n", v);
            vec_clear(&sample_buf);
            sample_neighbors(&sample_buf, &old_r[v], pk, false);
            heap_union(&old[v], &sample_buf);

            vec_clear(&sample_buf);
            sample_neighbors(&sample_buf, &new_r[v], pk, false);
            heap_union(&new[v], &sample_buf);
            for (int i = 0; i < new[v].size; i++) {
                for (int j = 0; j < new[v].size; j++) {
                    if (i == j) continue;
                    u1.id = new[v].arr[i].id;
                    u2.id = new[v].arr[j].id;
                    if (u1.id <= u2.id || heap_find_by_index(&old[v], u2.id) == -1) continue;
                    l = metric(data.values[u1.id], data.values[u2.id], data.dim);
                    c += nn_update(&B[u1.id], &u2);
                    c += nn_update(&B[u2.id], &u1);
                }
            }
        }
        printf("iteration complete: %d / %d\n", c, stop_iter);
    }

    printf("done, cleaning up...\n");

    nn_free_heap_list(old, data.size);
    nn_free_heap_list(old_r, data.size);
    nn_free_heap_list(new, data.size);
    nn_free_heap_list(new_r, data.size);

    return B;
}


int reverse_heap_list(vec_t* dst, vec_t* src, int size)
{
    int dst_index, src_index;
    node_t t;
    for (int v = 0; v < size; v++) {
        for (int u = 0; u < src[v].size; u++) {
            t = src[v].arr[u];
            dst_index = t.id;
            t.id = v;
            heap_insert(&dst[dst_index], &t);
        }
    }
}

int sample_neighbors(vec_t* dst, vec_t* src, int k, bool use_new)
{
    if (src->size == 0) {
        return 0;
    }

    int rand_index;
    for (int i = 0; i < src->size - 1; i++) {
        node_t t;
        rand_index = i + (rand() % (src->size - i));
        t = src->arr[rand_index];
        src->arr[rand_index] = src->arr[i];
        src->arr[i] = t;
    }

    int j = 0;
    for (int i = 0; i < src->size && j < k; i++) {
        if (use_new && !src->arr[i].new) continue;
        heap_insert(dst, &(src->arr[i]));
        if (use_new) src->arr[i].new = false;
        j++;
    }

    return j;
}

