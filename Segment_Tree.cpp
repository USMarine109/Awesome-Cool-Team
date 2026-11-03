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
void Segment_Tree<T>::query(int node, int start, int end, int left, int right)
{
    if (right < start || end < left)
    {
        return;
    }
        return tree[node];
}
template<typename T>
void Segment_Tree<T>::update(int index, T value)
{

}

