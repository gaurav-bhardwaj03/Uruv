//
// Created by gaurav on 31/03/22.
//

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

//const int64_t MAX = 32;
//const int64_t MIN = 8;

template<typename K, typename V>
class Node{
public:
    bool is_leaf;
    std::atomic<int64_t> help_idx;
    std::atomic<int64_t> count;
    std::atomic<int64_t> node_count;
    std::atomic<std::vector<std::pair<K,Vnode<V>*>>*> res;
    std::vector<K> key;
    std::atomic<Node<K,V>*> ptr[MAX+1];
    virtual void mark() = 0;
    virtual void create_new_leaf() = 0;
    virtual void split_internal(Node<K,V>*, Node<K,V>*) = 0;
    Node<K,V>* next = nullptr;
    std::atomic<Node<K,V>*> new_next;
    std::atomic<int64_t>status;
    Linked_List<K,V> data_array_list;
    virtual int64_t delete_leaf(K key) = 0;
    virtual int64_t insert_leaf(K,V) = 0;
    virtual void stabilize() = 0;
    virtual void split_leaf() = 0;
    virtual bool merge_leaf(Node<K,V>* right_child) = 0;
    virtual Node<K,V>* merge_internal(Node<K,V>* ,Node<K,V>*, int64_t) = 0;
};

#endif //LF_B_TREEV2_NODE_H
