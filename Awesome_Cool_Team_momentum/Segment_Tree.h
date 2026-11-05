// SegmentTree.h - range-sum segment tree written from scratch (no STL algorithms).
//
// Layout: iterative bottom-up tree stored in a single array of size 2n.
//   leaves live at tree[n .. 2n-1]  (tree[n + i] == a[i])
//   internal node p stores tree[2p] + tree[2p+1]
// This is the classic "non-recursive" segment tree; it works for any n // (not only powers of two) because sums are associative and the query // walks both boundaries upward independently.
//
// Complexity: build O(n), point update O(log n), range query O(log n), // memory 2n values.
//made by asher
#pragma once
#include <cstddef>
#include <stdexcept>
#include <vector>
template <typename T>
class SegmentTree {
public:
    SegmentTree() = default;
    explicit SegmentTree(const std::vector<T>& values) { build(values); }
    // Build in O(n): copy leaves then fill internal nodes from the bottom up.
    void build(const std::vector<T>& values) {
        n_ = values.size();
        tree_.assign(2 * n_, T{});
        for (std::size_t i = 0; i < n_; ++i) tree_[n_ + i] = values[i];
        for (std::size_t p = n_ - 1; p >= 1 && n_ > 0; --p) tree_[p] = tree_[2 * p] + tree_[2 * p + 1];
    }
    // a[i] = value, O(log n)
    void set(std::size_t i, T value) {
        check(i);
        std::size_t p = i + n_;
        tree_[p] = value;
        for (p >>= 1; p >= 1; p >>= 1) tree_[p] = tree_[2 * p] + tree_[2 * p + 1];
    }
    // a[i] += delta, O(log n)
    void add(std::size_t i, T delta) {
        check(i);
        for (std::size_t p = i + n_; p >= 1; p >>= 1) tree_[p] += delta;
    }
    // Sum of a[l..r] inclusive, O(log n). Returns 0 for an empty range (l > r).
    T query(std::size_t l, std::size_t r) const {
        if (l > r) return T{};
        check(r);
        T left{}, right{};
        std::size_t lo = l + n_, hi = r + n_ + 1;  // half-open [lo, hi)
        while (lo < hi) {
            if (lo & 1) left = left + tree_[lo++];
            if (hi & 1) right = tree_[--hi] + right;
            lo >>= 1;
            hi >>= 1;
        }
        return left + right;
    }
    T get(std::size_t i) const { check(i); return tree_[i + n_]; }
    std::size_t size() const { return n_; }
    std::size_t memoryBytes() const { return sizeof(*this) + tree_.capacity() * sizeof(T); }
private:
    void check(std::size_t i) const {
        if (i >= n_) throw std::out_of_range("SegmentTree index out of range");
    }
    std::size_t n_ = 0;
    std::vector<T> tree_;
};
