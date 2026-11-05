//
// Created by kenne on 11/1/2026.
//
#include "Segment_Tree.h"
#include <climits>
#include <iostream>
#include <vector>
#include<queue>
using namespace std;


template <typename T>
void Segment_Tree<T>::build(int node, int start, int end)
{
    if (start == end)
    {
        tree[node] = arr[start];
        return;

        int m = mid(start, end);
        build(left(node), start, m);
        build(right(node), m + 1, end);
        tree[node] = min(tree[left(node)], tree[right(node)]);
    }
}

template <typename T>
void Segment_Tree<T>::update(int node, int start, int end,int index, T value)
{
    if (start == end)
    {
        arr[start] = value;
        tree[node] = value;
        return;
    }
    int m = mid(start, end);
    if (index <= m)
        update(left(node), start, m, index, value);
    else
        update(right(node), m + 1, end, index, value);
    tree[node] = min(tree[left(node)], tree[right(node)]);
}
template <typename T>
T Segment_Tree<T>::query(int node, int start, int end, int left, int right)
{
    if (right < start || end < 1)
    {
        return INT_MAX;
    }
    if (left <= start && end <= right)
    {
        return tree[node];
    }
    int m = mid(start, end);
    return min(query(left(node), start, m, left, right), query(right(node), m + 1, end, left, right));
}
template<typename T>
void Segment_Tree<T>::update(int index, T value)
{
    update(1, 0, n - 1, index, value);
}

template <typename T>
T Segment_Tree<T>::query(int left, int right)
{
    return query(1, 0, n - 1, left, right);
}

