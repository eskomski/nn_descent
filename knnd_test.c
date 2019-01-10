#include "knnd.h"
#include "vec.h"
#include <stdio.h>
#include <math.h>

float l2(float* v1, float* v2, int d)
{
    float acc = 0.0f;
    for (int i = 0; i < d; i++) {
        acc += (v1[i] - v2[i]) * (v1[i] - v2[i]);
    }

    return sqrt(acc);
}

int main()
{
    dataset_t data;
    if(make_test_data(&data, 100, 10)) return 1;

    vec_t* B = nn_descent(data, &l2, 10, 1.0, 0.01);
    for (int i = 0; i < data.size; i++) {
        heap_print(&B[i]);
    }

    nn_free_heap_list(B, data.size);
    free(data.values);
}

