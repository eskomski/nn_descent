#include "vec.h"
#include <float.h>
#include <stdio.h>
#include <stdbool.h>

int vec_create(vec_t* v, int capacity)
{
    if (capacity == 0) return 0;

    v->size = 0;
    v->_capacity = capacity;
    v->arr = malloc(sizeof(node_t) * capacity);
    v->min = FLT_MAX;
    if (v->arr == NULL) return 0;

    return 1;
}

void vec_clear(vec_t* v) { if (v) v->size = 0; }

static int _vec_check_size(vec_t* h) {
    if (h->size + 1 > h->_capacity) {
        node_t* tmp;
        h->_capacity *= 2;
        tmp = realloc(h->arr, sizeof(node_t)*(h->_capacity));
        h->arr = tmp;

        if (!(h->arr)) return 0;
    }
    return 1;
}

int vec_insert(vec_t* h, node_t* node)
{
    if (!_vec_check_size(h)) return 0;
    h->arr[h->size++] = *node;
    return 1;
}

int heap_insert(vec_t* h, node_t* node)
{
    if (heap_find_by_index(h, node->id) != -1) return 0;
    if (!_vec_check_size(h)) return -1;

    // add new node, then bubble up
    h->arr[h->size] = *node;
    if (h->size != 0) {
        int n = h->size;
        int p = (n-1)/2;
        node_t t;
        while (p >= 0 && h->arr[p].val < h->arr[n].val) {
            t = h->arr[p];
            h->arr[p] = h->arr[n];
            h->arr[n] = t;
            n = p;
            p = (n-1)/2;
        }
    }


    h->min = node->val < h->min ? node->val : h->min;
    h->size++;
    return 1;
}

int heap_union(vec_t* h1, vec_t* h2)
{
    for (int i = 0; i < h2->size; i++) {
        if (heap_insert(h1, &(h2->arr[i])) == -1) return 0;
    }
    return 1;
}

int heap_find_by_index(vec_t* h, int index)
{
    for (int i = 0; i < h->size; i++) {
        if (h->arr[i].id == index) return i;
    }
    return -1;
}

void vec_free(vec_t* h) { if (h) free(h->arr); }

