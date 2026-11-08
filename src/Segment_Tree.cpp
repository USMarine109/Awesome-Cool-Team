//
// Created by kenne on 11/1/2026.
//
#include "Segment_Tree.h"
#include <climits>
#include <iostream>
#include <vector>
#include<queue>
#include <cstddef>
#include <stdexcept>
using namespace std;


//made by aidan
#include "Segment_Tree.h"
// Default constructor
template <typename T>
SegmentTree<T>::SegmentTree() = default;

// Constructor that builds the tree
template <typename T>
SegmentTree<T>::SegmentTree(
    const std::vector<T>& values) {

    build(values);
}

// Build the Segment Tree in O(n)
template <typename T>
void SegmentTree<T>::build(
    const std::vector<T>& values) {

    n_ = values.size();
    tree_.assign(2 * n_, T{});

    // Copy original values into leaf nodes
    for (std::size_t i = 0; i < n_; ++i) {
        tree_[n_ + i] = values[i];
    }

    // Calculate internal node sums
    for (std::size_t p = n_; p > 1;) {
        --p;

        tree_[p] = tree_[2 * p] +
                   tree_[2 * p + 1];
    }
}

// Replace value at index i
template <typename T>
void SegmentTree<T>::set(
    std::size_t i, T value) {

    check(i);

    std::size_t p = i + n_;
    tree_[p] = value;

    // Update parent nodes
    for (p >>= 1; p > 0; p >>= 1) {
        tree_[p] = tree_[2 * p] +
                   tree_[2 * p + 1];
    }
}

// Add delta to index i
template <typename T>
void SegmentTree<T>::add(
    std::size_t i, T delta) {

    check(i);

    for (std::size_t p = i + n_;
         p > 0;
         p >>= 1) {

        tree_[p] += delta;
    }
}

// Calculate range sum from l to r
template <typename T>
T SegmentTree<T>::query(
    std::size_t l, std::size_t r) const {

    if (l > r) {
        return T{};
    }

    check(l);
    check(r);

    T left{};
    T right{};

    std::size_t lo = l + n_;
    std::size_t hi = r + n_ + 1;

    while (lo < hi) {

        // Include right boundary of left side
        if (lo & 1) {
            left = left + tree_[lo];
            ++lo;
        }

        // Include left boundary of right side
        if (hi & 1) {
            --hi;
            right = tree_[hi] + right;
        }

        lo >>= 1;
        hi >>= 1;
    }

    return left + right;
}

// Get value at index i
template <typename T>
T SegmentTree<T>::get(
    std::size_t i) const {

    check(i);
    return tree_[i + n_];
}

// Return number of elements
template <typename T>
std::size_t SegmentTree<T>::size() const {

    return n_;
}

// Return estimated memory usage
template <typename T>
std::size_t SegmentTree<T>::memoryBytes() const {

    return sizeof(*this) +
           tree_.capacity() * sizeof(T);
}

// Check index validity
template <typename T>
void SegmentTree<T>::check(
    std::size_t i) const {

    if (i >= n_) {
        throw std::out_of_range(
            "SegmentTree index out of range");
    }
}

// Generate implementation for long long
template class SegmentTree<long long>;
template class SegmentTree<double>;

