// FenwickTree.h - Binary Indexed Tree (Fenwick 1994) written from scratch.
//
// Internally 1-indexed: bit[i] stores the sum of a[i - lowbit(i) + 1 .. i] // where lowbit(i) = i & -i. Public API is 0-indexed to match SegmentTree.
//
// Complexity: build O(n) (linear "push to parent" construction), // point update O(log n), prefix sum O(log n), range sum = two prefix sums, // memory n + 1 values.
//made by aidan
#pragma once

#include <cstddef>
#include <vector>

template <typename T>
class FenwickTree {
public:
    FenwickTree();
    explicit FenwickTree(const std::vector<T>& values);

    void build(const std::vector<T>& values);
    void add(std::size_t i, T delta);
    void set(std::size_t i, T value);

    T prefix(std::size_t i) const;
    T query(std::size_t l, std::size_t r) const;
    T get(std::size_t i) const;

    std::size_t size() const;
    std::size_t memoryBytes() const;

private:
    static std::size_t lowbit(std::size_t i);
    void check(std::size_t i) const;

    std::size_t n_ = 0;
    std::vector<T> bit_;
};
