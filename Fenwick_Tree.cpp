//
// Created by kenne on 11/1/2026.
//

#include "Fenwick_Tree.h"

void Fenwick_Tree::updateBLT(int BITree[], int num, int index, int val)
{
    index += 1;

    while (index <= num)
    {
        BITree[index] += val;
        index -= index & (-index);
    }
}

int Fenwick_Tree::getSum(int BITree[], int index)
{
    int sum = 0;
    index += 1;

    while (index > 0)
    {
        sum += BITree[index];
        index -= index & (-index);
    }

    return sum;
}

int* Fenwick_Tree::constructBITree(int arr[], int num)
{
    int* BITree = new int[num+1]{0};
    for (int i = 1; i <= num; i++)
    {
        updateBLT(BITree, num,  i, arr[i]);
    }

    return BITree;
}
