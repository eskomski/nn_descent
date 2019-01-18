# nn_descent

This is a basic C implementation of the approximate k-nearest neighbor algorithm
described in the paper "[Efficient K-Nearest Neighbor Graph Construction for
Generic Similarity Measures](http://www.cs.princeton.edu/cass/papers/www11.pdf)".
This was written as part of an implementation of the paper "[UMAP: Uniform Manifold
Approximation and Projection for Dimensionality Reduction](https://arxiv.org/abs/1802.03426)"
which is currently in progress.

As of yet, it's only been tested on random inputs using Euclidean distance, though
I plan to compare its output to that of a standard k-NN algorithm to ensure the
correctness of my implementation.

