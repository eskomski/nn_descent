#pragma once
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    int id;
    float val;
    bool new;
} node_t;

typedef struct {
    int size, _capacity;
    float min;
    node_t* arr;
} vec_t;

int vec_create(vec_t*, int);
void vec_clear();
void vec_free(vec_t*);

int vec_insert(vec_t*, node_t*);

int heap_insert(vec_t*, node_t*);
int heap_union(vec_t*, vec_t*);
int heap_find_by_index(vec_t*, int);

