//
// Created by kenne on 11/1/2026.
//
#include <iostream>
#include <vector>

using namespace std;
#ifndef AWESOME_COOL_TEAM_MOMENTUM_FENWICK_TREE_H
#define AWESOME_COOL_TEAM_MOMENTUM_FENWICK_TREE_H


class Fenwick_Tree
{
    void updateBLT(int BITree[], int num, int index, int val);
    int getSum(int BITree[], int index);
    int* constructBITree(int arr[], int num);

};


#endif //AWESOME_COOL_TEAM_MOMENTUM_FENWICK_TREE_H
