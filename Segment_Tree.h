//
// Created by kenne on 11/1/2026.
//

#include <climits>
#include <iostream>
#include <vector>
using namespace std;

#ifndef AWESOME_COOL_TEAM_MOMENTUM_SEGMENT_TREE_H
#define AWESOME_COOL_TEAM_MOMENTUM_SEGMENT_TREE_H


template <typename T>
class Segment_Tree
{
    vector<T> tree;
    vector<T> arr;
    int n;

    int left(int node)
    {
        return node * 2 + 1;
    }

    int right(int node)
    {
        return node * 2 + 2;
    }

    int mid(int left,int right)
    {
        return (left + (right - 1)) / 2;
    }

    void build(int node, int start, int end);
    void update(int node, int start, int end, int index, T value);
    T query(int node, int start, int end, int left, int right);
    void update(int index, T value);

public:
    Segment_Tree(const vector<T>& a)
        : arr(a)
        , n(a.size())
    {
        tree.resize(4 * n);
    }
    void update(int node, int start, int end,int index, T value);
    T query(int left, int right);

    //https://www.geeksforgeeks.org/cpp/segment-tree-in-cpp/ segamet tree format

};


#endif //AWESOME_COOL_TEAM_MOMENTUM_SEGMENT_TREE_H
