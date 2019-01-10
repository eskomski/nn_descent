#pragma once
#include "vec.h"
#include <stdbool.h>

// TODO: maybe use bit flags for bool argument in sample_neighbors?

typedef struct {
    int size;
    int dim;
    float** values; // be sure to cast to float or double types
} dataset_t;

int make_test_data(dataset_t*, int, int);
vec_t* heap_list_create(int, int);
void heap_list_free(vec_t*, int);
vec_t* nn_descent(dataset_t, float(*)(float*, float*, int), int, float, float);
int sample_neighbors(vec_t*, vec_t*, int, bool);
int update_nn(vec_t*, int, float);
int reverse_heap_list(vec_t*, vec_t*, int);

