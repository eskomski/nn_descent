#include "knnd.h"
#include "vec.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

float l2(float* v1, float* v2, int d)
{
    float acc = 0.0f;
    for (int i = 0; i < d; i++) {
        acc += (v1[i] - v2[i]) * (v1[i] - v2[i]);
    }

    return sqrt(acc);
}

void heap_check(vec_t* h, int i)
{
    int l = i*2 + 1, r = i*2 + 2;
    if (l < h->_capacity) {
        assert(h->arr[i].val >= h->arr[l].val);
        heap_check(h, l);
    }

    if (r < h->_capacity) {
        assert(h->arr[i].val >= h->arr[r].val);
        heap_check(h, r);
    }
}

int main()
{
    dataset_t data;
    if (make_test_data(&data, 1000, 10)) return 1;

    vec_t* B = nn_descent(data, &l2, 20, 1.0, 0.001);

    for (int i = 0; i < data.size; i++) {
        heap_check(&B[i], 0);
    }

    heap_list_free(B, data.size);
    free(data.values);
}

