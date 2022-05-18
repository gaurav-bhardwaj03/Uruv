
#ifndef UNTITLED_INTERNAL_NODE_H
#define UNTITLED_INTERNAL_NODE_H

#include "Node.h"

template<typename K, typename V>
class internal_node: public Node<int64_t, int64_t>
{
public:
    std::atomic<int64_t> help_idx;
    std::vector<K> key;
    std::atomic<Node<K,V>*> ptr[MAX + 1];
    void mark();
    void split_internal(internal_node<K,V>*, internal_node<K,V>*);
    internal_node<K,V>* merge_internal(internal_node<K,V>* ,internal_node<K,V>*, int64_t);
    internal_node<K,V> (int min, int max)
    {
        this -> min = min;
        this-> max = max;
        help_idx = -1;
        count.store(0, std::memory_order_seq_cst);
        status = 0;
        key.resize(max);
        is_leaf = false;
        for(int64_t i = 0; i <= max; i++)
            ptr[i].store(nullptr, std::memory_order_seq_cst);
    }
};

template<typename K, typename V>
void internal_node<K,V>::mark(){
    if(status > 0)
        return;
    for(int64_t i = 0; i <= max; i++)
    {
        while(true){
            Node<K,V>* curr_node = ptr[i].load(std::memory_order_seq_cst);
            if(is_marked((uintptr_t) curr_node))
                break;
            Node<K,V>* marked_node = (Node<K,V>*) set_mark((uintptr_t) curr_node);
            if(ptr[i].compare_exchange_strong(curr_node, marked_node, std::memory_order_seq_cst, std::memory_order_seq_cst))
                break;
        }
    }
    if(status == 0){
        int64_t temp = 0;
        status.compare_exchange_strong(temp,temp + 1, std::memory_order_seq_cst, std::memory_order_seq_cst);
    }
}

template<typename K, typename V>
void internal_node<K,V> ::split_internal(internal_node<K,V>* left_child, internal_node<K,V>* right_child) {
    left_child -> count.store(count.load(std::memory_order_seq_cst)/2, std::memory_order_seq_cst);
    right_child -> count.store(count.load(std::memory_order_seq_cst) - left_child -> count.load(std::memory_order_seq_cst) - 1);
    int64_t j = 0;
    int64_t k = 0;
    int64_t i;
    for(i = 0; i < left_child -> count.load(std::memory_order_seq_cst); i++)
    {
        left_child -> key[i] = key[j++];
        left_child -> ptr[i] = (Node<K,V>*) unset_mark ((uintptr_t) ptr[k++].load(std::memory_order_seq_cst));
    }
    left_child -> ptr[i] = (Node<K,V>*) unset_mark ((uintptr_t) ptr[k++].load(std::memory_order_seq_cst));
    j++;
    for(i = 0; i < right_child -> count.load(std::memory_order_seq_cst); i++)
    {
        right_child -> key[i] = key[j++];
        right_child -> ptr[i] = (Node<K,V>*) unset_mark ((uintptr_t) ptr[k++].load(std::memory_order_seq_cst));
    }
    right_child -> ptr[i] = (Node<K,V>*) unset_mark ((uintptr_t) ptr[k++].load(std::memory_order_seq_cst));
}

template<typename K, typename V>
internal_node<K,V>* internal_node<K,V>::merge_internal(internal_node<K,V>* curr_node,internal_node<K,V>* right_child, int64_t idx) {
    if(count.load(std::memory_order_seq_cst) + right_child -> count.load(std::memory_order_seq_cst) < max) // Merge
    {
        internal_node<K,V>* new_node = new internal_node<K,V> (curr_node->min, curr_node->max);
        internal_node<K,V>* new_merged_node = new internal_node<K,V> (min, max);
        new_node -> count.store(curr_node -> count.load(std::memory_order_seq_cst) - 1, std::memory_order_seq_cst);
        new_merged_node -> count.store(count.load(std::memory_order_seq_cst) + right_child -> count.load(std::memory_order_seq_cst) + 1, std::memory_order_seq_cst);
        int64_t i = 0;
        for( ;i < count.load(std::memory_order_seq_cst); i++)
        {
            new_merged_node -> key [i] = key[i];
            new_merged_node -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) ptr[i].load(std::memory_order_seq_cst)));
        }
        new_merged_node -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) ptr[i].load(std::memory_order_seq_cst)));
        new_merged_node -> key[i] = curr_node -> key[idx];
        i++;
        for(int64_t j = 0; j < right_child -> count.load(std::memory_order_seq_cst); j++)
        {
            new_merged_node -> key[i] = right_child -> key[j];
            new_merged_node -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) right_child -> ptr[j].load(std::memory_order_seq_cst)));
            i++;
        }
        new_merged_node -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) right_child -> ptr[right_child -> count.load(std::memory_order_seq_cst)].load(std::memory_order_seq_cst)));
        for(i = 0; i < idx; i++)
        {
            new_node -> key[i] = curr_node -> key[i];
            new_node -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) curr_node -> ptr[i].load(std::memory_order_seq_cst)));
        }
        new_node -> ptr[i].store(new_merged_node);
        for(; i < curr_node -> count.load(std::memory_order_seq_cst) - 1; i++)
        {
            new_node -> key[i] = curr_node -> key[i + 1];
            new_node -> ptr[i + 1].store((Node<K,V>*)(unset_mark((uintptr_t) curr_node -> ptr[i + 2].load(std::memory_order_seq_cst))));
        }
        return new_node;
    }
    else // borrow
    {
        std::chrono::high_resolution_clock::time_point op_time;
        internal_node<K,V>* left_child = new internal_node<K,V> (min, max);
        internal_node<K,V>* new_right_child = new internal_node<K,V> (min, max);
        internal_node<K,V>* new_node = new internal_node<K,V> (curr_node -> min, curr_node -> max);
        new_node -> count.store(curr_node -> count.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
        if(count.load(std::memory_order_seq_cst) > min) // left borrow
        {
            left_child -> count.store(count.load(std::memory_order_seq_cst) - 1);
            new_right_child -> count.store(right_child -> count.load(std::memory_order_seq_cst) + 1, std::memory_order_seq_cst);
            for(int64_t i = 0; i < left_child -> count.load(std::memory_order_seq_cst); i++)
            {
                left_child -> key[i] = key[i];
                left_child -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) ptr[i].load(std::memory_order_seq_cst)));
            }
            left_child -> ptr[left_child -> count.load(std::memory_order_seq_cst)].store((Node<K,V>*) unset_mark((uintptr_t) ptr[left_child -> count].load(std::memory_order_seq_cst)));
            new_right_child -> key[0] = curr_node -> key[idx];
            new_right_child -> ptr[0].store((Node<K,V>*) unset_mark((uintptr_t) ptr[count.load(std::memory_order_seq_cst)].load(std::memory_order_seq_cst)));
            for(int64_t i = 1; i < new_right_child -> count.load(std::memory_order_seq_cst); i++)
            {
                new_right_child -> key[i] = right_child -> key[i-1];
                new_right_child -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) right_child -> ptr[i-1].load(std::memory_order_seq_cst)));
            }
            new_right_child -> ptr[new_right_child -> count.load(std::memory_order_seq_cst)].store((Node<K,V>*) unset_mark((uintptr_t) right_child -> ptr[right_child -> count].load(std::memory_order_seq_cst)));
            int64_t i = 0;
            for(; i < idx; i++)
            {
                new_node -> key[i] = curr_node -> key[i];
                new_node -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) curr_node -> ptr[i].load(std::memory_order_seq_cst)));
            }
            new_node -> ptr[i].store(left_child);
            new_node -> key[i] = key[count.load(std::memory_order_seq_cst) - 1];
            i++;
            new_node -> ptr[i].store(new_right_child);
            for(; i <= curr_node -> count.load(std::memory_order_seq_cst); i++)
            {
                new_node -> key[i] = curr_node -> key[i];
                new_node -> ptr[i + 1].store((Node<K,V>*) unset_mark((uintptr_t) curr_node -> ptr[i + 1].load(std::memory_order_seq_cst)));
            }
            return new_node;
        }
        else{ //right borrow
            std::chrono::high_resolution_clock::time_point op_time;
            left_child -> count.store(count.load(std::memory_order_seq_cst) + 1, std::memory_order_seq_cst);
            new_right_child -> count.store(right_child -> count.load(std::memory_order_seq_cst) - 1);
            int64_t i = 0;
            for( ;i < count.load(std::memory_order_seq_cst); i++)
            {
                left_child -> key[i] = key[i];
                left_child -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) ptr[i].load(std::memory_order_seq_cst)));
            }
            left_child -> key[i] = curr_node -> key[idx];
            left_child -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) ptr[i].load(std::memory_order_seq_cst)));
            left_child -> ptr[i + 1].store((Node<K,V>*) unset_mark((uintptr_t) right_child -> ptr[0].load(std::memory_order_seq_cst)));
            for(i = 1; i < right_child -> count; i++)
            {
                new_right_child -> key[i - 1] = right_child -> key[i];
                new_right_child -> ptr[i - 1].store((Node<K,V>*) unset_mark((uintptr_t) right_child -> ptr[i].load(std::memory_order_seq_cst)));
            }
            new_right_child -> ptr[i - 1].store((Node<K,V>*) unset_mark((uintptr_t) right_child -> ptr[i].load(std::memory_order_seq_cst)));
            i = 0;
            for(; i < idx; i++)
            {
                new_node -> key[i] = curr_node -> key[i];
                new_node -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) curr_node -> ptr[i].load(std::memory_order_seq_cst)));
            }
            new_node -> ptr[i].store(left_child);
            new_node -> key[i] = right_child -> key[0];
            i++;
            new_node -> ptr[i].store(new_right_child);
            for(; i <= curr_node -> count; i++)
            {
                new_node -> key[i] = curr_node -> key[i];
                new_node -> ptr[i + 1].store((Node<K,V>*) unset_mark((uintptr_t) curr_node -> ptr[i + 1].load(std::memory_order_seq_cst)));
            }
            return new_node;
        }
    }
}


#endif //UNTITLED_INTERNAL_NODE_H
