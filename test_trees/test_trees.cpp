//
// Created by kenne on 11/7/2026.
//
//unit testing for Segment_Tree and Fenwick_Tree
#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <random>
using namespace std;

#include "Fenwick_Tree.h"
#include "Load_Dataset.h"
#include "Segment_Tree.h"

static int failures = 0, checks = 0;

#define CHECK(cond) \
    do { \
        ++checks; \
        if (!(cond)) { \
            ++failures; \
            std::cerr << "Check failed: " << #cond << std::endl; \
        } \
    } while (0)

static long long int naive(const vector<long long>& a, size_t n, size_t k)
{
    long long int sum = 0;
    for (size_t i = n; i < k; ++i) {
        sum += a[i];
    }
    return sum;
}

static void testAllRangesSmall() {
    std::mt19937 rng(1);
    std::uniform_int_distribution<int> v(-6, 6);
    for (std::size_t n = 1; n <= 70; ++n) {
        std::vector<long long> a(n);
        for (auto& x : a) x = v(rng);
        SegmentTree<long long> s(a);
        FenwickTree<long long> f(a);
        bool ok = true;
        for (std::size_t l = 0; l < n; ++l)
            for (std::size_t r = l; r < n; ++r) {
                long long e = naive(a, l, r);
                ok = ok && s.query(l, r) == e && f.query(l, r) == e;
            }
        CHECK(ok);
    }
}

// Random interleaving of point updates (add + set) and range queries vs naive.
static void testRandomUpdates() {
    std::mt19937 rng(2026);
    for (std::size_t n : {1u, 2u, 3u, 17u, 1000u, 4096u, 10007u}) {
        std::uniform_int_distribution<std::size_t> idx(0, n - 1);
        std::uniform_int_distribution<int> v(-6, 6), op(0, 2);
        std::vector<long long> a(n);
        for (auto& x : a) x = v(rng);
        SegmentTree<long long> s(a);
        FenwickTree<long long> f(a);
        bool ok = true;
        for (int it = 0; it < 20000; ++it) {
            int o = op(rng);
            if (o == 0) {
                std::size_t i = idx(rng); long long d = v(rng);
                a[i] += d; s.add(i, d); f.add(i, d);
            } else if (o == 1) {
                std::size_t i = idx(rng); long long nv = v(rng);
                a[i] = nv; s.set(i, nv); f.set(i, nv);
            } else {
                std::size_t l = idx(rng), r = idx(rng);
                if (l > r) std::swap(l, r);
                long long e = naive(a, l, r);
                ok = ok && s.query(l, r) == e && f.query(l, r) == e;
            }
        }
        for (std::size_t i = 0; i < n; ++i) ok = ok && s.get(i) == a[i] && f.get(i) == a[i];
        CHECK(ok);
    }
}

static void testBuildMatchesIncremental() {
    std::mt19937 rng(7);
    std::uniform_int_distribution<int> v(-100, 100);
    std::vector<long long> a(12345);
    for (auto& x : a) x = v(rng);
    FenwickTree<long long> linear(a), incremental(std::vector<long long>(a.size(), 0));
    for (std::size_t i = 0; i < a.size(); ++i) incremental.add(i, a[i]);
    bool ok = true;
    for (std::size_t i = 0; i < a.size(); i += 97) ok = ok && linear.prefix(i) == incremental.prefix(i);
    CHECK(ok);
    SegmentTree<double> sd(std::vector<double>{0.5, 1.0, -1.0, 3.0});
    CHECK(sd.query(0, 3) == 3.5);
    CHECK(sizeof(long long) * 2 * a.size() <= SegmentTree<long long>(a).memoryBytes());
    CHECK(linear.memoryBytes() < SegmentTree<long long>(a).memoryBytes());
}

static void testMomentumFormula() {
    Event e; e.period = 1;
    e.type = "Shot"; e.outcome = "Goal";            CHECK(momentum::halfPoints(e) == 6);
    e.outcome = "Saved";                            CHECK(momentum::halfPoints(e) == 2);
    e.outcome = "Saved to Post";                    CHECK(momentum::halfPoints(e) == 2);
    e.outcome = "Off T";                            CHECK(momentum::halfPoints(e) == 0);
    e.type = "Own Goal For"; e.outcome = "";        CHECK(momentum::halfPoints(e) == 6);
    e.type = "Pass"; e.x = 60; e.endX = 90;         CHECK(momentum::halfPoints(e) == 1);
    e.outcome = "Incomplete";                       CHECK(momentum::halfPoints(e) == 0);
    e.outcome = ""; e.x = 100; e.endX = 110;        CHECK(momentum::halfPoints(e) == 0);
    e.assist = 1;                                   CHECK(momentum::halfPoints(e) == 1);
    e.type = "Dispossessed"; e.assist = 0;          CHECK(momentum::halfPoints(e) == -2);
    e.type = "Miscontrol";                          CHECK(momentum::halfPoints(e) == -2);
    e.type = "Dribble"; e.outcome = "Incomplete";   CHECK(momentum::halfPoints(e) == -2);
    e.type = "Shot"; e.outcome = "Goal"; e.period = 5; CHECK(momentum::halfPoints(e) == 0);  // shootout
}

int main() {
    testEmptyAndSingle();
    testKnownValues();
    testAllRangesSmall();
    testRandomUpdates();
    testBuildMatchesIncremental();
    testMomentumFormula();
    std::cout << (failures ? "TESTS FAILED: " : "All tests passed: ") << checks - failures << "/" << checks
              << " checks\n";
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}



