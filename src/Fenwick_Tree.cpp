//
// Created by kenne on 11/1/2026.
//

//made by aidan
#include "Fenwick_Tree.h"

#include <cstddef>
#include <stdexcept>
#include <vector>

// Default constructor
template <typename T>
FenwickTree<T>::FenwickTree() = default;

// Constructor that builds the tree
template <typename T>
FenwickTree<T>::FenwickTree(
    const std::vector<T>& values) {
    build(values);
}

// Build Fenwick Tree in O(n)
template <typename T>
void FenwickTree<T>::build(
    const std::vector<T>& values) {

    n_ = values.size();
    bit_.assign(n_ + 1, T{});

    for (std::size_t i = 1; i <= n_; ++i) {
        bit_[i] += values[i - 1];

        std::size_t parent = i + lowbit(i);

        if (parent <= n_) {
            bit_[parent] += bit_[i];
        }
    }
}

// Add delta to an element
template <typename T>
void FenwickTree<T>::add(
    std::size_t i, T delta) {

    check(i);

    for (std::size_t k = i + 1;
         k <= n_;
         k += lowbit(k)) {

        bit_[k] += delta;
    }
}

// Replace an element's value
template <typename T>
void FenwickTree<T>::set(
    std::size_t i, T value) {

    add(i, value - get(i));
}

// Calculate prefix sum from 0 to i
template <typename T>
T FenwickTree<T>::prefix(std::size_t i) const {

    check(i);

    T sum{};

    for (std::size_t k = i + 1;
         k > 0;
         k -= lowbit(k)) {

        sum += bit_[k];
    }

    return sum;
}

// Calculate range sum from l to r
template <typename T>
T FenwickTree<T>::query(
    std::size_t l, std::size_t r) const {

    if (l > r) {
        return T{};
    }

    check(l);
    check(r);

    if (l == 0) {
        return prefix(r);
    }

    return prefix(r) - prefix(l - 1);
}

// Get value at index i
template <typename T>
T FenwickTree<T>::get(std::size_t i) const {

    return query(i, i);
}

// Return number of elements
template <typename T>
std::size_t FenwickTree<T>::size() const {

    return n_;
}

// Return estimated memory usage
template <typename T>
std::size_t FenwickTree<T>::memoryBytes() const {

    return sizeof(*this) +
           bit_.capacity() * sizeof(T);
}

// Find lowest set bit
template <typename T>
std::size_t FenwickTree<T>::lowbit(
    std::size_t i) {

    return i & (~i + 1);
}

// Check valid index
template <typename T>
void FenwickTree<T>::check(
    std::size_t i) const {

    if (i >= n_) {
        throw std::out_of_range(
            "FenwickTree index out of range");
    }
}

// Generate implementation for long long
template class FenwickTree<long long>;
template class FenwickTree<double>;
