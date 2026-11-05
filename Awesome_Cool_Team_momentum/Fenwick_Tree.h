// FenwickTree.h - Binary Indexed Tree (Fenwick 1994) written from scratch.
//
// Internally 1-indexed: bit[i] stores the sum of a[i - lowbit(i) + 1 .. i] // where lowbit(i) = i & -i. Public API is 0-indexed to match SegmentTree.
//
// Complexity: build O(n) (linear "push to parent" construction), // point update O(log n), prefix sum O(log n), range sum = two prefix sums, // memory n + 1 values.
//made by asher
#pragma once
#include <cstddef>
#include <stdexcept>
#include <vector>

template <typename T>
class FenwickTree {
public:
    FenwickTree() = default;
    explicit FenwickTree(const std::vector<T>& values) { build(values); }

    // O(n) build: each node pushes its partial sum to its parent i + lowbit(i).
    void build(const std::vector<T>& values) {
        n_ = values.size();
        bit_.assign(n_ + 1, T{});
        for (std::size_t i = 1; i <= n_; ++i) {
            bit_[i] += values[i - 1];
            std::size_t parent = i + lowbit(i);
            if (parent <= n_) bit_[parent] += bit_[i];
        }
    }

    // a[i] += delta, O(log n)
    void add(std::size_t i, T delta) {
        check(i);
        for (std::size_t k = i + 1; k <= n_; k += lowbit(k)) bit_[k] += delta;
    }

    // a[i] = value, O(log n) (needs the current value: one range query + one add)
    void set(std::size_t i, T value) { add(i, value - get(i)); }

    // Sum of a[0..i] inclusive, O(log n)
    T prefix(std::size_t i) const {
        check(i);
        T s{};
        for (std::size_t k = i + 1; k > 0; k -= lowbit(k)) s += bit_[k];
        return s;
    }

    // Sum of a[l..r] inclusive, O(log n). Returns 0 for an empty range (l > r).
    T query(std::size_t l, std::size_t r) const {
        if (l > r) return T{};
        return l == 0 ? prefix(r) : prefix(r) - prefix(l - 1);
    }

    T get(std::size_t i) const { return query(i, i); }
    std::size_t size() const { return n_; }
    std::size_t memoryBytes() const { return sizeof(*this) + bit_.capacity() * sizeof(T); }

private:
    static std::size_t lowbit(std::size_t i) { return i & (~i + 1); }
    void check(std::size_t i) const {
        if (i >= n_) throw std::out_of_range("FenwickTree index out of range");
    }
    std::size_t n_ = 0;
    std::vector<T> bit_;
};
