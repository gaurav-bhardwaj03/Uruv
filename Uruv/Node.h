
#ifndef LF_B_TREEV2_NODE_H
#define LF_B_TREEV2_NODE_H

#include <mutex>
#include <fstream>
#include <unordered_map>
#include <chrono>
#include<thread>
#include <iostream>
#include "util.h"
#include "LF_LL.h"

template<typename K, typename V>
class Node{
public:
    bool is_leaf;
    int max;
    int min;
    std::atomic<int64_t> count;
    virtual void mark() = 0;
    std::atomic<int64_t>status; // Variable to check whether node is already marked or stabilize
};

#endif //LF_B_TREEV2_NODE_H
