//
// Created by gaurav on 31/03/22.
//

#ifndef UNTITLED_LF_LL_H
#define UNTITLED_LF_LL_H

#include<atomic>
#include <limits>
#include<vector>
#include <unordered_set>
#include <iostream>
#include "util.h"

template<typename K, typename V>
class Linked_List {
public:
    ll_Node<K,V>* head;
    Linked_List(){
        ll_Node<K,V>* max = new ll_Node<K,V>(std::numeric_limits<K>::max(), 0);
        ll_Node<K,V>* min = new ll_Node<K,V>(std::numeric_limits<K>::min(), 0);
        min -> next.store(max);
        head = min;
    }
    int64_t count();
    void mark();
    int insert(K Key, V value);
    bool insert(K Key, V value, ll_Node<K,V>* new_node);
    int insert(K Key, V value, int tid, int phase);
    bool search(K key);
    std::vector<std::pair<K,Vnode<V>*>>* collect(std::vector<std::pair<K,Vnode<V>*>>*, int64_t);
    void range_query(int64_t low, int64_t high, int64_t curr_ts, std::vector<std::pair<K,V>>& res);
    ll_Node<K,V>* find(K key);
    ll_Node<K,V>* find(K key, ll_Node<K,V>**);
    void init_ts(Vnode<V> *node){
        if(node -> ts.load(std::memory_order_seq_cst) == -1){
            int64_t invalid_ts = -1;
            int64_t global_ts = version_tracker.get_latest_timestamp();
            node -> ts.compare_exchange_strong(invalid_ts, global_ts, std::memory_order_seq_cst, std::memory_order_seq_cst );
        }
    }
    V read(ll_Node<K,V> *node) {
        init_ts(node -> vhead);
        return node -> vhead.load(std::memory_order_seq_cst) -> value;
    }

    bool vCAS(ll_Node<K, V> *node, V old_value,  V new_value){
        Vnode<V>* head = (Vnode<V>*) unset_mark((uintptr_t) node -> vhead.load(std::memory_order_seq_cst));
        init_ts(head);
        if(head -> value != old_value) return false;
        if(head -> value == new_value)
            return true;
        Vnode<V>* new_node = new Vnode<V>(new_value, head);
        if(expt_sleep){
            int dice_roll = dice(gen);
            if(dice_roll < 5){
                std::this_thread::sleep_for(std::chrono::microseconds(5));
            }
        }
        if(node -> vhead.compare_exchange_strong(head,new_node, std::memory_order_seq_cst, std::memory_order_seq_cst))
        {
            init_ts(new_node);
            return true;
        }
        else{
//            delete new_node;
            return false;
        }
    }
    bool vCAS(ll_Node<K, V> *node, V old_value,  V new_value, Vnode<V>* new_node, Vnode<V>* head, Vnode<V>* vnext){
        if(head -> value != old_value) return false;
        if(head -> value == new_value)
            return true;
        if(!new_node -> nextv.compare_exchange_strong(vnext, head, std::memory_order_seq_cst, std::memory_order_seq_cst))
            return false;
        if(expt_sleep){
            int dice_roll = dice(gen);
            if(dice_roll < 5){
                std::this_thread::sleep_for(std::chrono::microseconds(5));
            }
        }
        if(node -> vhead.compare_exchange_strong(head,new_node, std::memory_order_seq_cst, std::memory_order_seq_cst))
        {
            init_ts(new_node);
            return true;
        }
        else{
//            delete new_node;
            return false;
        }
    }
};

template<typename K, typename V>
void Linked_List<K,V>::mark() {
    ll_Node<K,V>* left_node = head;
    while(left_node -> next.load(std::memory_order_seq_cst)){
        if(!is_marked_ref((long) left_node -> next.load(std::memory_order_seq_cst)))
        {
            ll_Node<K,V>* curr_next = left_node -> next.load(std::memory_order_seq_cst);
            left_node -> next.compare_exchange_strong(curr_next, (ll_Node<K,V>*)set_mark((long) curr_next));
        }
        if(!is_marked_ref((long) left_node -> vhead.load(std::memory_order_seq_cst)))
        {
            Vnode<V>* curr_vhead = left_node -> vhead.load(std::memory_order_seq_cst);
            left_node ->  vhead.compare_exchange_strong(curr_vhead, (Vnode<V>*)set_mark((long) curr_vhead));
        }
        left_node = (ll_Node<K,V>*)get_unmarked_ref((long) left_node -> next.load(std::memory_order_seq_cst));
    }
}

template<typename K, typename V>
std::vector<std::pair<K,Vnode<V>*>>* Linked_List<K,V>::collect(std::vector<std::pair<K,Vnode<V>*>> *res, int64_t min_ts){
    ll_Node<K,V>* left_node = ( ll_Node<K,V>* ) get_unmarked_ref((long)head -> next.load(std::memory_order_seq_cst));
    ll_Node<K,V>* left_next = (ll_Node<K,V>*) get_unmarked_ref((long) left_node -> next.load(std::memory_order_seq_cst));
    while(left_next){
        Vnode<V> *left_node_vhead = (Vnode<V>*) get_unmarked_ref((uintptr_t)left_node -> vhead.load(std::memory_order_seq_cst));
        if(!(left_node_vhead -> value == -1 && left_node_vhead -> ts < min_ts)){
            (*res).push_back(std::make_pair(left_node -> key, left_node_vhead));
        }
        left_node = left_next;
        left_next = ( ll_Node<K,V>* ) get_unmarked_ref((long) left_next -> next.load(std::memory_order_seq_cst));
    }
    return res;
}

template<typename K, typename V>
void Linked_List<K,V>::range_query(int64_t low, int64_t high, int64_t curr_ts, std::vector<std::pair<K,V>> &res){
    ll_Node<K,V>* left_node = ( ll_Node<K,V>* ) get_unmarked_ref((long) head -> next.load(std::memory_order_seq_cst));
    ll_Node<K,V>* left_next = (ll_Node<K,V>*) get_unmarked_ref((long) left_node -> next.load(std::memory_order_seq_cst));
    while(left_next && left_node -> key < low){
        left_node = left_next;
        left_next = ( ll_Node<K,V>* ) get_unmarked_ref((long) left_next -> next.load(std::memory_order_seq_cst));
    }
    while(left_next && left_node -> key <= high)
    {
        Vnode<V>* curr_vhead = (Vnode<V>*) get_unmarked_ref((uintptr_t)left_node -> vhead.load(std::memory_order_seq_cst));
        init_ts(curr_vhead);
        while(curr_vhead && curr_vhead -> ts > curr_ts) curr_vhead = curr_vhead -> nextv;
        if(curr_vhead && curr_vhead  -> value != -1)
            res.push_back(std::make_pair(left_node -> key, curr_vhead -> value));
        left_node = left_next;
        left_next = ( ll_Node<K,V>* ) get_unmarked_ref((long) left_next -> next.load(std::memory_order_seq_cst));
    }
}

template<typename K, typename V>
bool Linked_List<K,V>::search(K key) {
    ll_Node<K,V>* curr = head;
    while(curr -> key < key)
        curr = (ll_Node<K,V>*) get_unmarked_ref((long) curr -> next .load(std::memory_order_seq_cst));
    return (curr -> key == key && !is_marked_ref((long) curr -> next.load(std::memory_order_seq_cst)));
}



template<typename K, typename V>
ll_Node<K,V>* Linked_List<K,V>::find(K key) {
    ll_Node<K, V> *left_node = head;
    while(true) {
        ll_Node<K, V> *right_node = left_node -> next.load(std::memory_order_seq_cst);
        if (is_marked_ref((long) right_node))
            return nullptr;
        if (right_node->key >= key)
            return right_node;
        (left_node) = right_node;
    }
}

template<typename K, typename V>
ll_Node<K,V>* Linked_List<K,V>::find(K key, ll_Node<K, V> **left_node) {
    (*left_node) = head;
    while(true) {
        ll_Node<K, V> *right_node = (*left_node) -> next.load(std::memory_order_seq_cst);
        if (is_marked_ref((long) right_node)) {
            return nullptr;
        }
        if (right_node -> key >= key)
            return right_node;
        (*left_node) = right_node;
    }
}

template<typename K, typename V>
int Linked_List<K,V>::insert(K key, V value) {
//    return -1;
    while(true)
    {
        ll_Node<K,V>* prev_node = nullptr;
        ll_Node<K,V>* right_node = find(key, &prev_node);
        if(right_node == nullptr)
            return -1;
        if(right_node -> key == key){
            while(true)
            {
                V curr_value = read(right_node);
                if(curr_value == value)
                    return 0;
                if(vCAS(right_node,curr_value,value))
                    break;
                else if(is_marked_ref((uintptr_t) right_node -> vhead.load(std::memory_order_seq_cst)))
                    return -1;
                FAILURE++;
                if(FAILURE >= MAX_FAILURE)
                    return -1;
            }
            return 0;
        }
        else
        {
            if(value == -1)
                return 0;
            ll_Node<K,V>* new_node = new ll_Node<K,V>(key,value);
            new_node -> next.store(right_node);
            if(expt_sleep){
                int dice_roll = dice(gen);
                if(dice_roll < 5){
                    std::this_thread::sleep_for(std::chrono::microseconds(5));
                }
            }
            if(prev_node -> next.compare_exchange_strong(right_node, new_node)) {
                init_ts(new_node -> vhead);
                return 1;
            }
            FAILURE++;
            if(FAILURE >= MAX_FAILURE)
                return -1;
        }
    }
}

template<typename K, typename V>
int Linked_List<K,V>::insert(K key, V value, int tid, int phase) {
    while(true)
    {
        ll_Node<K,V>* prev_node = nullptr;
        ll_Node<K,V>* right_node = find(key, &prev_node);
        if(right_node == nullptr)
            return -1;
        if(right_node -> key == key){
            while(true)
            {
                State<K,V>* curr_state = stateArray[tid];
                Vnode<V>* new_Vnode = curr_state -> vnode;
                Vnode<V>* new_next = new_Vnode -> nextv.load();
                Vnode<V>* currVhead = (Vnode<V>*)get_unmarked_ref((long)right_node -> vhead.load(std::memory_order_seq_cst));
                init_ts(currVhead);
                V curr_value = read(right_node);
                if(curr_state -> phase > phase || curr_state -> vnode ->ts != -1) {
                    curr_state->finished = true;
                    return 0;
                }
                if(curr_value == value)
                    return 0;
                if(curr_state -> finished)
                    return 0;
                if(vCAS(right_node,curr_value,value, new_Vnode, currVhead, new_next)) {
                    curr_state -> finished = true;
                    break;
                }
                else if(is_marked_ref((uintptr_t) right_node -> vhead.load(std::memory_order_seq_cst)))
                    return -1;
            }
            return 0;
        }
        else
        {
            State<K,V>* curr_state = stateArray[tid];
            if(curr_state -> phase != phase || curr_state -> vnode ->ts != -1)
            {
                curr_state -> finished = true;
                return 0;
            }
            if(expt_sleep){
                int dice_roll = dice(gen);
                if(dice_roll < 5){
                    std::this_thread::sleep_for(std::chrono::microseconds(5));
                }
            }
            ll_Node<K,V>* new_node = new ll_Node<K,V>(key, curr_state -> vnode, right_node);
            if(prev_node -> next.compare_exchange_strong(right_node, new_node)) {
                init_ts(new_node -> vhead);
                curr_state -> finished = true;
                return 1;
            }
            else
            {
                if(curr_state -> phase != phase || curr_state -> vnode -> ts != -1)
                {
                    curr_state -> finished = true;
                    return 0;
                }
            }
        }
    }
}

//template<typename K, typename V>
//int64_t Linked_List<K,V>::count() {
//    int64_t count = 0;
//    ll_Node<K,V>* left_node = head -> next.load(std::memory_order_seq_cst);
//    while(left_node -> next.load(std::memory_order_seq_cst)){
//        if(!is_marked_ref((long) left_node -> next.load(std::memory_order_seq_cst)))
//            count++;
//        left_node = ( ll_Node<K,V>* ) get_unmarked_ref((long) left_node -> next.load(std::memory_order_seq_cst));
//    }
//    return count;
//}


#endif //UNTITLED_LF_LL_H
