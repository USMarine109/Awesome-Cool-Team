//
// Created by kenne on 11/1/2026.
//

#include <climits>
#include <iostream>
#include <vector>
#include <cstddef>
using namespace std;

#ifndef AWESOME_COOL_TEAM_MOMENTUM_SEGMENT_TREE_H
#define AWESOME_COOL_TEAM_MOMENTUM_SEGMENT_TREE_H

// SegmentTree.h - range-sum segment tree written from scratch (no STL algorithms).
//
// Layout: iterative bottom-up tree stored in a single array of size 2n.
//   leaves live at tree[n .. 2n-1]  (tree[n + i] == a[i])
//   internal node p stores tree[2p] + tree[2p+1]
// This is the classic "non-recursive" segment tree; it works for any n // (not only powers of two) because sums are associative and the query // walks both boundaries upward independently.
//
// Complexity: build O(n), point update O(log n), range query O(log n), // memory 2n values.
//made by aidan

template <typename T>
class SegmentTree {
public:
    SegmentTree();
    explicit SegmentTree(const std::vector<T>& values);

    void build(const std::vector<T>& values);
    void set(std::size_t i, T value);
    void add(std::size_t i, T delta);

    T query(std::size_t l, std::size_t r) const;
    T get(std::size_t i) const;

    std::size_t size() const;
    std::size_t memoryBytes() const;

private:
    void check(std::size_t i) const;

    std::size_t n_ = 0;
    std::vector<T> tree_;
};



#endif //AWESOME_COOL_TEAM_MOMENTUM_SEGMENT_TREE_H
