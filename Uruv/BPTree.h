//
// Created by gaurav on 31/03/22.
//

#ifndef LF_B_TREEV2_BPTREE_H
#define LF_B_TREEV2_BPTREE_H

#include "Leaf_node.h"

template<typename K, typename V>
class BPTree{
public:
    std::atomic<Node<K,V>*> root;
    std::atomic<int64_t> size;
    std::vector<std::pair<K,V>>collect();
    std::vector<std::pair<K,V>> range_query(int64_t low, int64_t high);
    Node<K,V>* do_helping(Node<K,V>*, int64_t, Node<K,V>*);
    V search(K key);
    Node<K,V>* merge(Node<K,V>* left_child, Node<K,V>* right_child, Node<K,V>* curr_node, int64_t idx, Node<K,V>* prev_node, int64_t prev_idx);
    BPTree<K,V> (int threads)
    {
        root.store(new leaf_node<K,V>());
        for(int i = 0; i < threads; i++)
            stateArray.push_back(nullptr);
        NUM_THREADS = threads;
        size = 0;
    }
    bool remove(K, int tid, int phase);
    bool insert(K, V, int tid, int phase);
    void remove(K, int);
    void insert(K, V, int);
    bool remove(K);
    void insert(K, V);
    Node<K,V>* remove_parent(Node<K,V>*, Node<K,V>*, int64_t);
    Node<K,V>* remove_parent(Node<K,V>*, Node<K,V>*, Node<K,V>* ,int64_t);

    void init_thread(int tid);
};

template<typename K, typename V>
V BPTree<K,V>:: search(K key){
    Node<K,V>* curr_node = (Node<K,V>*) unset_mark((uintptr_t) root.load(std::memory_order_seq_cst));
    if(curr_node == nullptr)
        return -1;
    while(!curr_node -> is_leaf)
    {
        int64_t low = 0, high = curr_node->count.load(std::memory_order_seq_cst) - 1;
        while (low < high) {
            int64_t mid = low + (high - low) / 2;
            if (key >= curr_node->key[mid]) {
                low = mid + 1;
            } else high = mid;
        }
        if (curr_node->key[low] <= key) {
            curr_node = (Node<K,V>*) unset_mark((uintptr_t) curr_node ->ptr[curr_node->count.load(std::memory_order_seq_cst)].load(std::memory_order_seq_cst));
        } else {
            curr_node = (Node<K,V>*) unset_mark((uintptr_t) curr_node->ptr[low].load(std::memory_order_seq_cst));
        }
    }
    return curr_node -> data_array_list.search(key);
}

template<typename K, typename V>
void BPTree<K,V>::init_thread(int tid)
{
    help_obj = new HelpRecord(tid);
}

template<typename K, typename V>
void BPTree<K,V>::remove(K key, int tid)
{
    FAILURE = 0;
    if(!remove(key, tid, -1))
    {
        FAILURE = 0;
        int phase = phase_counter.fetch_add(1);
        State<K,V>* new_obj = new State<K,V>(phase, key, -1);
        stateArray[tid] = new_obj;
        remove(key, tid, phase);
    }
    help_obj -> nextCheck--;
    if(help_obj -> nextCheck == 0){
        State<K,V>* curr_state = stateArray[help_obj -> curr_tid];
        if((curr_state) && curr_state -> phase == help_obj -> lastPhase) {
            if(!curr_state -> finished)
            {
                if(curr_state -> value == -1)
                {
                    remove(curr_state -> key, help_obj -> curr_tid, curr_state -> phase);
                }
                else
                {
                    insert(curr_state -> key, curr_state -> value, help_obj->curr_tid, curr_state -> phase);
                }
            }
        }
        help_obj -> reset();
    }
}

template<typename K, typename V>
void BPTree<K,V>::insert(K key, V value, int tid)
{
    FAILURE = 0;
    if(!insert(key,value, tid, -1))
    {
        FAILURE = 0;
        int phase = phase_counter.fetch_add(1);
        State<K,V>* new_obj = new State<K,V>(phase, key, value);
        stateArray[tid] = new_obj;
        insert(key,value, tid, phase);
    }
    help_obj -> nextCheck--;
    if(help_obj -> nextCheck == 0){
        State<K,V>* curr_state = stateArray[help_obj -> curr_tid];
        if((curr_state) && curr_state -> phase == help_obj -> lastPhase) {
            if(!curr_state -> finished)
            {
                if(curr_state -> value == -1)
                {
                    remove(curr_state -> key, help_obj -> curr_tid, curr_state -> phase);
                }
                else
                {
                    insert(curr_state -> key, curr_state -> value, help_obj -> curr_tid, curr_state -> phase);
                }
            }
        }
        help_obj -> reset();
    }
}


template<typename K, typename V>
bool BPTree<K,V>::insert(K key, V value, int tid, int phase) {
    retry:
    if(phase == -1 && FAILURE >= MAX_FAILURE)
        return false;
    if(phase == -1)
        FAILURE++;
    Node<K,V>* curr_node = (Node<K,V>*) unset_mark((uintptr_t) root.load(std::memory_order_seq_cst));
    if(curr_node == nullptr)
    {
        leaf_node<K,V>* new_node = new leaf_node<K,V>();
        new_node -> insert_leaf(key,value);
        if(root.compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
            size++;
            return true;
        }
        else {
            delete new_node;
            goto retry;
        }
    }
    else if(curr_node -> is_leaf)
    {
        if(curr_node -> status > 0)
        {
            curr_node -> mark();
            curr_node -> stabilize();
            if(curr_node -> count.load(std::memory_order_seq_cst) >= MAX)
            {
                curr_node -> split_leaf();
                Node<K,V>* new_node = new internal_node<K,V> ();
                new_node -> count.store(1, std::memory_order_seq_cst);
                Node<K,V>* left_child = curr_node -> new_next.load(std::memory_order_seq_cst);
                Node<K,V>* right_child = left_child -> next;
                new_node -> ptr[0].store(left_child, std::memory_order_seq_cst);
                new_node -> ptr[1].store(right_child, std::memory_order_seq_cst);
                new_node -> key[0] = right_child -> data_array_list.head -> next.load(std::memory_order_seq_cst) -> key;
                if(!root.compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
                    delete new_node;
                    goto retry;
                }
                curr_node = new_node;
            }
            else
            {
                curr_node -> create_new_leaf();
                if(!root.compare_exchange_strong(curr_node, curr_node -> new_next.load(std::memory_order_seq_cst), std::memory_order_seq_cst, std::memory_order_seq_cst)) {
                    goto retry;
                }
                curr_node = curr_node -> new_next.load(std::memory_order_seq_cst);
            }
        }
    }
    else if(curr_node -> count.load(std::memory_order_seq_cst) >= MAX) // if root is an Internal node and it's full
    {
        curr_node -> mark();
        Node<K,V>* left_child = new internal_node<K,V> ();
        Node<K,V>* right_child = new internal_node<K,V> ();
        curr_node -> split_internal (left_child, right_child);
        Node<K,V>* new_node = new internal_node<K,V> ();
        new_node -> ptr[0].store(left_child, std::memory_order_seq_cst);
        new_node -> ptr[1].store(right_child, std::memory_order_seq_cst);
        new_node -> key[0] = curr_node -> key[left_child -> count.load(std::memory_order_seq_cst)];
        new_node -> count.store(1, std::memory_order_seq_cst);
        if(!root.compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst))
        {
            delete new_node;
            delete right_child;
            delete left_child;
            goto retry;
        }
        curr_node = new_node;
    }
    Node<K,V>* prev_node = nullptr;
    int64_t prev_idx = -1;
    while(!curr_node -> is_leaf)
    {
        if(curr_node -> help_idx.load(std::memory_order_seq_cst) != -1)
        {
            Node<K,V>* res = do_helping(prev_node, prev_idx, curr_node); // todo
            if(!res)
                goto retry;
            else
                curr_node = res;
        }
        Node<K, V>* curr_child = nullptr;
        int64_t curr_idx = -1;
        int64_t low = 0, high = curr_node->count.load(std::memory_order_seq_cst) - 1;
        while (low < high) {
            int64_t mid = low + (high - low) / 2;
            if (key >= curr_node->key[mid]) {
                low = mid + 1;
            } else high = mid;
        }
        if (curr_node->key[low] <= key) {
            curr_child = (Node<K,V>*) unset_mark((uintptr_t) curr_node ->ptr[curr_node->count.load(std::memory_order_seq_cst)].load(std::memory_order_seq_cst));
            curr_idx = curr_node->count.load(std::memory_order_seq_cst);
        } else {
            curr_child = (Node<K,V>*) unset_mark((uintptr_t) curr_node->ptr[low].load(std::memory_order_seq_cst));
            curr_idx = low;
        }
        if(curr_child -> is_leaf && curr_child -> status.load(std::memory_order_seq_cst) > 0)
        {
            curr_node -> mark();
            curr_child -> stabilize();
			int curr_child_count = curr_child -> count.load(std::memory_order_seq_cst);
            std::vector<std::pair<K, Vnode<V>*>> *res1 = curr_child -> res.load(std::memory_order_seq_cst);
			if(curr_node -> help_idx.load(std::memory_order_seq_cst) == -1)
            {
                int64_t temp = -1;
                if(!curr_node -> help_idx.compare_exchange_strong(temp, curr_idx, std::memory_order_seq_cst, std::memory_order_seq_cst))
                    goto retry;
            }
            else
                goto retry;
            if((int64_t) curr_child -> count.load(std::memory_order_seq_cst) >= MAX) //Split the Current Child
            {
                curr_child -> split_leaf();
                Node<K,V>* new_node = new internal_node<K,V>();
                Node<K,V>* left_child = curr_child -> new_next.load(std::memory_order_seq_cst);
                Node<K,V>* right_child = left_child -> next;
                int64_t x = right_child -> data_array_list.head -> next.load(std::memory_order_seq_cst) -> key;
                bool k = false;
                int64_t key_idx = 0;
                int64_t curr_ptr_id = 0;
                int64_t new_ptr_id = 0;
                new_node -> count.store(curr_node -> count.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
                for (int64_t i = 0; i < new_node -> count.load(std::memory_order_seq_cst); ++i) {
                    if (!k && curr_node -> key[key_idx] > x) {
                        new_node -> key[i] = x;
                        new_node -> ptr[new_ptr_id++].store(left_child, std::memory_order_seq_cst);
                        new_node -> ptr[new_ptr_id++].store(right_child, std::memory_order_seq_cst);
                        k = true;
                        curr_ptr_id++;
                        new_node -> count++;
                    } else {
                        new_node -> key[i] = curr_node -> key[key_idx++];
                        new_node -> ptr[new_ptr_id++].store((Node<K,V>* ) unset_mark((uintptr_t) curr_node -> ptr[curr_ptr_id++].load(std::memory_order_seq_cst)));
                    }
                }
                if(!k) {
					int new_node_count = new_node -> count.load(std::memory_order_seq_cst);
                    new_node -> key[new_node_count] = x;
                    new_node -> ptr[new_node_count].store(left_child, std::memory_order_seq_cst);
                    new_node -> count++;
                    new_node -> ptr[new_node_count + 1].store(right_child, std::memory_order_seq_cst);
                }
                Node<K,V>* t = curr_node;
                if (prev_node == nullptr) {
                    if (!root.compare_exchange_strong(curr_node, new_node, std:: memory_order_seq_cst, std::memory_order_seq_cst)) {
                        delete new_node;
                        goto retry;
                    }
                }
                else {
                    if (!prev_node -> ptr[prev_idx].compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
                        delete new_node;
                        goto retry;
                    }
                }
                if(key < x)
                    curr_child = left_child;
                else {
                    curr_child = right_child;
                    curr_idx++;
                }
                curr_node = new_node;
            }
            else if((int64_t)curr_child -> count.load(std::memory_order_seq_cst) < MIN)  // Merge the current child
            {
                int64_t ls_idx = curr_idx - 1; // Left_Sibling Index
                int64_t rs_idx = curr_idx + 1; // Right Sibling Index
                if(ls_idx >= 0)        // Left Sibling Exist
                {
                    Node<K,V>* left_child = (Node<K,V>*) unset_mark((uintptr_t )curr_node -> ptr[ls_idx].load(std::memory_order_seq_cst));
                    Node<K,V>* temp = nullptr;
                    left_child -> mark();
                    left_child -> stabilize();
                    Node<K, V>* res = merge(left_child, curr_child, curr_node, ls_idx, prev_node, prev_idx);
                    if(!res)
                        goto retry;
                    else {
                        curr_node = res;
                        continue;
                    }
                }
                else if(rs_idx <= curr_node -> count.load(std::memory_order_seq_cst))
                {
                    Node<K,V>* right_child = (Node<K,V>*) unset_mark((uintptr_t )curr_node -> ptr[rs_idx].load(std::memory_order_seq_cst));
                    Node<K,V>* temp = nullptr;
                    right_child -> mark();
                    right_child -> stabilize();
                    Node<K, V>* res = merge(curr_child, right_child, curr_node, curr_idx, prev_node, prev_idx);
                    if(!res)
                        goto retry;
                    else {
                        curr_node = res;
                        continue;
                    }
                }
            }
            else{      // Replace the current child
                curr_child -> create_new_leaf();
                Node<K,V>* new_node = new internal_node<K,V> ();
                new_node -> count.store(curr_node -> count.load(std::memory_order_seq_cst));
                int64_t i;
                for(i = 0; i < curr_node -> count.load(std::memory_order_seq_cst); i++)
                {
                    new_node -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) curr_node -> ptr[i].load(std::memory_order_seq_cst)));
                    new_node -> key[i] = curr_node -> key[i];
                }
                new_node -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) curr_node -> ptr[i].load(std::memory_order_seq_cst)));
                new_node -> ptr[curr_idx].store(curr_child -> new_next.load(std::memory_order_seq_cst));
                if(prev_node == nullptr)
                {
                    if(!root.compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
                        delete new_node;
                        goto retry;
                    }
                    else {
                        curr_node = new_node;
                        curr_child = curr_child -> new_next.load(std::memory_order_seq_cst);
                    }
                }
                else
                {
                    if(!prev_node -> ptr[prev_idx].compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
                        delete new_node;
                        goto retry;
                    }
                    else {
                        curr_node = new_node;
                        curr_child = curr_child -> new_next.load(std::memory_order_seq_cst);
                    }
                }
            }
        }
        else if(!curr_child -> is_leaf && curr_child -> count.load(std::memory_order_seq_cst) >= MAX) // if curr_child is not leaf
        {
            curr_node -> mark();
            if(curr_node -> help_idx.load(std::memory_order_seq_cst) == -1)
            {
                int64_t temp = -1;
                if(!curr_node -> help_idx.compare_exchange_strong(temp, curr_idx, std::memory_order_seq_cst, std::memory_order_seq_cst))
                    goto retry;
            }
            else
                goto retry;
            curr_child -> mark();
            Node<K,V>* right_child = new internal_node<K,V> ();
            Node<K,V>* left_child = new internal_node<K,V> ();
            curr_child -> split_internal(left_child, right_child);
            Node<K,V>* new_node = new internal_node<K,V> ();
            int64_t x = curr_child -> key[left_child -> count.load(std::memory_order_seq_cst)];
            new_node -> count.store(curr_node -> count.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
            bool k = false;
            K key_idx = 0;
            int64_t curr_ptr_id = 0;
            int64_t new_ptr_id = 0;
            for (int64_t i = 0; i < new_node -> count.load(std::memory_order_seq_cst); ++i) {
                if (!k && curr_node->key[key_idx] > x) {
                    new_node -> key[i] = x;
                    new_node -> ptr[new_ptr_id++] = left_child;
                    new_node -> ptr[new_ptr_id++] = right_child;
                    k = true;
                    curr_ptr_id++;
                    new_node -> count++;
                } else {
                    new_node -> key[i] = curr_node -> key[key_idx++];
                    new_node -> ptr[new_ptr_id++].store((Node<K,V>* ) unset_mark((uintptr_t) curr_node->ptr[curr_ptr_id++].load(std::memory_order_seq_cst)));
                }
            }
            if (!k) {
				int new_node_count = new_node -> count.load(std::memory_order_seq_cst);
                new_node->key[new_node_count] = x;
                new_node->ptr[new_node_count] = left_child;
                new_node->count++;
                new_node->ptr[new_node_count + 1] = right_child;
            }
            if (prev_node == nullptr) {
                if (!root.compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst)) {
                    delete new_node;
                    delete left_child;
                    delete right_child;
                    goto retry;
                }
            }
            else {
                if (!prev_node->ptr[prev_idx].compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst)) {
                    delete new_node;
                    delete left_child;
                    delete right_child;
                    goto retry;
                }
            }
            if(key < x)
                curr_child = left_child;
            else {
                curr_child = right_child;
                curr_idx++;
            }
            curr_node = new_node;
        }
        prev_node = curr_node;
        curr_node = curr_child;
        prev_idx = curr_idx;
    }
    int64_t res;
    if(phase == -1)
        res = ((leaf_node<K,V>*)curr_node) -> insert_leaf(key, value);
    else
        res = ((leaf_node<K,V>*)curr_node) -> insert_leaf(key, value, tid, phase);

    if(res == 0) {
        goto retry;
    }
    else if(res == 1)
        size++;
    return true;
}

template<typename K, typename V>
bool BPTree<K,V>::remove(K key, int tid, int phase) {
    retry:
    if(phase == -1 && FAILURE >= MAX_FAILURE)
        return false;
    if(phase == -1)
        FAILURE++;
    Node<K,V>* curr_node = (Node<K,V>*) unset_mark((uintptr_t) root.load(std::memory_order_seq_cst));
    Node<K,V>* old_curr_node = curr_node;
    if(curr_node == nullptr){
        return false;
    }
    if(curr_node -> is_leaf)
    {
        if(curr_node -> status == 0) {
            int64_t result = curr_node -> delete_leaf(key);
            if (result == 1) {
                size--;
                return true;
            } else if (result == 0) {
                goto retry;
            } else {
                return false;
            }
        }
        else
        {
            curr_node -> mark();
            curr_node -> stabilize();
            if(curr_node -> count.load(std::memory_order_seq_cst) >= MAX)
            {
                curr_node -> split_leaf();
                Node<K,V>* new_node = new internal_node<K,V> ();
                new_node -> count.store(1, std::memory_order_seq_cst);
                Node<K,V>* left_child = curr_node -> new_next.load(std::memory_order_seq_cst);
                Node<K,V>* right_child = left_child -> next;
                new_node -> ptr[0] = left_child;
                new_node -> ptr[1] = right_child;
                new_node -> key[0] = right_child -> data_array_list.head -> next.load(std::memory_order_seq_cst) -> key;
                root.compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst);
                curr_node = new_node;
                goto retry;
            }
            else
            {
                curr_node -> create_new_leaf();
                root.compare_exchange_strong(curr_node, curr_node -> new_next.load(std::memory_order_seq_cst), std::memory_order_seq_cst, std::memory_order_seq_cst);
                curr_node = curr_node -> new_next.load(std::memory_order_seq_cst);
                goto retry;
            }
        }
    }
    Node<K,V>* prev_node = nullptr;
    Node<K,V>* old_prev_node = nullptr;
    Node<K,V>* old_curr_child = nullptr;
    int64_t prev_idx = -1;
    while(!curr_node -> is_leaf)
    {
        if(curr_node -> help_idx.load(std::memory_order_seq_cst) != -1)
        {
            Node<K,V>* res = do_helping(prev_node, prev_idx, curr_node);
            if(!res)
                goto retry;
            else
                curr_node = res;
        }
        Node<K, V>* curr_child = nullptr;
        int64_t curr_idx = -1;
        int64_t low = 0, high = curr_node->count.load(std::memory_order_seq_cst) - 1;
        while (low < high) {
            int64_t mid = low + (high - low) / 2;
            if (key >= curr_node->key[mid]) {
                low = mid + 1;
            } else high = mid;
        }
        if (curr_node->key[low] <= key) {
            curr_child = (Node<K,V>*) unset_mark((uintptr_t) curr_node ->ptr[curr_node->count.load(std::memory_order_seq_cst)].load(std::memory_order_seq_cst));
            curr_idx = curr_node->count.load(std::memory_order_seq_cst);
        } else {
            curr_child = (Node<K,V>*) unset_mark((uintptr_t) curr_node->ptr[low].load(std::memory_order_seq_cst));
            curr_idx = low;
        }
        if(curr_child -> is_leaf && curr_child -> status > 0)
        {
            curr_node -> mark();
            curr_child -> mark();
            curr_child -> stabilize();
            if(curr_node -> help_idx.load(std::memory_order_seq_cst) == -1)
            {
                int64_t temp = -1;
                if(!curr_node -> help_idx.compare_exchange_strong(temp, curr_idx, std::memory_order_seq_cst, std::memory_order_seq_cst))
                    goto retry;
            }
            else {
                goto retry;
            }
            if(curr_child -> count.load(std::memory_order_seq_cst) >= MAX) //Split the Current Child
            {
                curr_child -> split_leaf();
                Node<K,V>* new_node = new internal_node<K,V>();
                Node<K,V>* left_child = curr_child -> new_next;
                Node<K,V>* right_child = left_child -> next;
                int64_t x = right_child -> data_array_list.head -> next.load(std::memory_order_seq_cst) -> key;
                bool k = false;
                int64_t key_idx = 0;
                int64_t curr_ptr_id = 0;
                int64_t new_ptr_id = 0;
                new_node -> count.store(curr_node -> count.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
                for (int64_t i = 0; i < new_node -> count.load(std::memory_order_seq_cst); ++i) {
                    if (!k && curr_node -> key[key_idx] > x) {
                        new_node -> key[i] = x;
                        new_node -> ptr[new_ptr_id++] = left_child;
                        new_node -> ptr[new_ptr_id++] = right_child;
                        k = true;
                        curr_ptr_id++;
                        new_node -> count++;
                    } else {
                        new_node -> key[i] = curr_node -> key[key_idx++];
                        new_node -> ptr[new_ptr_id++].store((Node<K,V>* ) unset_mark((uintptr_t) curr_node -> ptr[curr_ptr_id++].load(std::memory_order_seq_cst)));
                    }
                }
                if(!k) {
                    new_node -> key[new_node -> count.load(std::memory_order_seq_cst)] = x;
                    new_node -> ptr[new_node -> count.load(std::memory_order_seq_cst)] = left_child;
                    new_node -> count++;
                    new_node -> ptr[new_node -> count.load(std::memory_order_seq_cst)] = right_child;
                }
                Node<K,V>* t = curr_node;
                if (prev_node == nullptr) {
                    if (!root.compare_exchange_strong(curr_node, new_node, std:: memory_order_seq_cst, std::memory_order_seq_cst)) {
                        delete new_node;
                        goto retry;
                    }
                }
                else {
                    if (!prev_node -> ptr[prev_idx].compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
                        delete new_node;
                        goto retry;
                    }
                }
                if(key < x)
                    curr_child = left_child;
                else {
                    curr_child = right_child;
                    curr_idx++;
                }
                curr_node = new_node;
            }
            else if(curr_child -> count < MIN)  // Merge the current child
            {
                int64_t ls_idx = curr_idx - 1; // Left_Sibling Index
                int64_t rs_idx = curr_idx + 1; // Right Sibling Index
                if(ls_idx >= 0)        // Left Sibling Exist
                {
                    Node<K,V>* left_child = (Node<K,V>*) unset_mark((uintptr_t )curr_node -> ptr[ls_idx].load(std::memory_order_seq_cst));
                    Node<K,V>* temp = nullptr;
                    left_child -> mark();
                    left_child -> stabilize();
                    Node<K, V>* res = merge(left_child, curr_child, curr_node, ls_idx, prev_node, prev_idx);
                    if(!res)
                        goto retry;
                    else {
                        curr_node = res;
                        continue;
                    }
                }
                else if(rs_idx <= curr_node -> count)
                {
                    Node<K,V>* right_child = (Node<K,V>*) unset_mark((uintptr_t )curr_node -> ptr[rs_idx].load(std::memory_order_seq_cst));
                    Node<K,V>* temp = nullptr;
                    right_child -> mark();
                    right_child -> stabilize();
                    Node<K, V>* res = merge(curr_child, right_child, curr_node, curr_idx, prev_node, prev_idx);
                    if(!res)
                        goto retry;
                    else {
                        curr_node = res;
                        continue;
                    }
                }
            }
            else{      // Replace the current child
                curr_child -> create_new_leaf();
                Node<K,V>* new_node = new internal_node<K,V> ();
                new_node -> count.store(curr_node -> count);
                int64_t i;
                for(i = 0; i < curr_node -> count; i++)
                {
                    new_node -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) curr_node -> ptr[i].load(std::memory_order_seq_cst)));
                    new_node -> key[i] = curr_node -> key[i];
                }
                new_node -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) curr_node -> ptr[i].load(std::memory_order_seq_cst)));
                new_node -> ptr[curr_idx].store(curr_child -> new_next.load(std::memory_order_seq_cst));
                Node<K,V>* t = curr_child -> new_next.load(std::memory_order_seq_cst);
                if(prev_node == nullptr)
                {
                    if(!root.compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst))
                        goto retry;
                    else {
                        curr_node = new_node;
                        curr_child = curr_child -> new_next.load(std::memory_order_seq_cst);
                    }
                }
                else
                {
                    if(!prev_node -> ptr[prev_idx].compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst))
                        goto retry;
                    else {
                        curr_node = new_node;
                        curr_child = curr_child -> new_next.load(std::memory_order_seq_cst);
                    }
                }
            }
        }
        else if(!curr_child -> is_leaf && curr_child -> count < MIN){
            curr_child -> mark();
            if(curr_node -> help_idx.load(std::memory_order_seq_cst) == -1)
            {
                int64_t temp = -1;
                if(!curr_node -> help_idx.compare_exchange_strong(temp, curr_idx, std::memory_order_seq_cst, std::memory_order_seq_cst))
                    goto retry;
            }
            else {
                goto retry;
            }
            curr_child -> mark();
            int64_t pc_idx = curr_idx - 1;
            int64_t nc_idx = curr_idx + 1;
            if(pc_idx >= 0)  //Left Child exist
            {
                Node<K,V>* left_child = (Node<K,V>*) unset_mark((uintptr_t)curr_node -> ptr[pc_idx].load(std::memory_order_seq_cst));
                left_child -> mark();
                bool res = merge(left_child, curr_child, curr_node, pc_idx, prev_node, prev_idx);
                if(res)
                    continue;
                else
                    goto retry;
            }
            else if(nc_idx <= curr_node -> count )
            {
                Node<K,V>* right_child = (Node<K,V>*) unset_mark((uintptr_t )curr_node -> ptr[nc_idx].load(std::memory_order_seq_cst));
                right_child -> mark();
                bool res = merge(curr_child, right_child, curr_node, curr_idx, prev_node, prev_idx);
                if(res)
                    continue;
                else
                    goto retry;
            }
        }
        prev_node = curr_node;
        old_prev_node = old_curr_node;
        curr_node = curr_child;
        old_curr_node = old_curr_child;
        prev_idx = curr_idx;
    }
    int64_t res;
    if(phase == -1)
        res = ((leaf_node<K,V>*)curr_node) -> delete_leaf(key);
    else
        res = ((leaf_node<K,V>*)curr_node) -> delete_leaf(key, tid, phase);
    if(res == 1){
        size--;
        return true;
    }
    else if(res == 0) {
        goto retry;
    }
    return true;
}


template<typename K, typename V>
Node<K,V>* BPTree<K,V>::merge(Node<K,V>* left_child, Node<K,V>* right_child, Node<K,V>* curr_node, int64_t idx, Node<K,V>* prev_node, int64_t prev_idx)
{
    if(left_child -> is_leaf) {
        bool merged = left_child -> merge_leaf(right_child);
        if (merged) {
            Node<K,V>* new_node = remove_parent(curr_node, left_child -> new_next, idx);
            if (new_node -> count == 0 && prev_node == nullptr) {
                root.compare_exchange_strong(curr_node, left_child -> new_next, std::memory_order_seq_cst, std::memory_order_seq_cst);
                return nullptr;
            } else if (prev_node == nullptr) {
                if (!root.compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
                    return nullptr;
                } else {
                    return new_node;
                }
            } else {
                if (!prev_node->ptr[prev_idx].compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
                    return nullptr;
                } else {
                    return new_node;
                }
            }
        } else {
            Node<K,V>* new_node = remove_parent(curr_node, left_child -> new_next.load(std::memory_order_seq_cst), left_child -> new_next.load(std::memory_order_seq_cst) -> next, idx);
            if (prev_node == nullptr) {
                root.compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst);
                return nullptr;
            } else {
                if (!prev_node -> ptr[prev_idx].compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
                    return nullptr;
                } else {
                    return new_node;
                }
            }
        }
    }
    else
    {
        Node<K,V>* new_node = left_child -> merge_internal(curr_node, right_child, idx);
        if(new_node -> count == 0 && prev_node == nullptr)
        {
            if(!root.compare_exchange_strong(curr_node, new_node -> ptr[0].load(std::memory_order_seq_cst)))
                return nullptr;
            else{
                return new_node -> ptr[0].load(std::memory_order_seq_cst);
            }
        }
        else if(prev_node == nullptr){
            if(!root.compare_exchange_strong(curr_node,new_node))
                return nullptr;
            else{
                return new_node;
            }
        }
        else {
            if (!prev_node->ptr[prev_idx].compare_exchange_strong(curr_node, new_node))
                return nullptr;
            else{
                return new_node;
            }
        }
    }
}

template<typename K, typename V>
Node<K,V>* BPTree<K,V>::remove_parent(Node<K,V>* curr_node, Node<K,V>* left_child, int64_t idx) {
    Node<K,V>* new_node = new internal_node<K,V> ();
    new_node -> count = curr_node -> count - 1;
    int64_t i = 0;
    for (; i < idx; i++) {
        new_node -> key[i] = curr_node->key[i];
        new_node -> ptr[i].store((Node<K,V>* ) unset_mark((uintptr_t) curr_node->ptr[i].load(std::memory_order_seq_cst)));
    }
    new_node -> ptr[i].store(left_child);
    for (; i < new_node -> count; i++) {
        new_node->key[i] = curr_node->key[i + 1];
        new_node->ptr[i + 1].store((Node<K,V>* ) unset_mark((uintptr_t) curr_node->ptr[i + 2].load(std::memory_order_seq_cst)));
    }
    return new_node;
}

template<typename K, typename V>
Node<K,V>* BPTree<K,V>::remove_parent(Node<K,V>* curr_node, Node<K,V>* left_child , Node<K,V>* right_child, int64_t idx) {
    Node<K,V>* new_node = new internal_node<K,V> ();
    new_node -> count.store(curr_node -> count);
    int64_t i = 0;
    for (; i < idx; i++) {
        new_node -> key[i] = curr_node -> key[i];
        new_node -> ptr[i].store((Node<K,V>* ) unset_mark((uintptr_t) curr_node -> ptr[i].load(std::memory_order_seq_cst)));
    }
    new_node -> ptr[i].store(left_child, std::memory_order_seq_cst);
    new_node -> key[i] = right_child -> data_array_list.head -> next.load(std::memory_order_seq_cst) -> key;
    i++;
    new_node -> ptr[i].store(right_child, std::memory_order_seq_cst);
    for(; i < new_node -> count; i++)
    {
        new_node -> key[i] = curr_node -> key[i];
        new_node -> ptr[i + 1].store((Node<K,V>* ) unset_mark((uintptr_t) curr_node -> ptr[i + 1].load(std::memory_order_seq_cst)));
    }
    return new_node;
}

template<typename K, typename V>
Node<K, V>* BPTree<K,V> :: do_helping(Node<K, V>* prev_node, int64_t prev_idx, Node<K, V>* curr_node) {
    int64_t curr_idx = curr_node -> help_idx.load(std::memory_order_seq_cst);
    Node<K,V>* curr_child = (Node<K,V>*) unset_mark((uintptr_t) curr_node -> ptr[curr_idx].load(std::memory_order_seq_cst));
    if(curr_child -> is_leaf) {
        if (curr_child -> count >= MAX) //Split the Current Child
        {
            curr_child -> split_leaf();
            Node<K,V>* new_node = new internal_node<K,V>();
            Node<K,V>* left_child = curr_child -> new_next.load(std::memory_order_seq_cst);
            Node<K,V>* right_child = left_child -> next;
            int64_t x = right_child -> data_array_list.head -> next.load(std::memory_order_seq_cst) -> key;
            bool k = false;
            int64_t key_idx = 0;
            int64_t curr_ptr_id = 0;
            int64_t new_ptr_id = 0;
            new_node -> count.store(curr_node -> count, std::memory_order_seq_cst);
            for (int64_t i = 0; i < new_node -> count; ++i) {
                if (!k && curr_node -> key[key_idx] > x) {
                    new_node -> key[i] = x;
                    new_node -> ptr[new_ptr_id++] = left_child;
                    new_node -> ptr[new_ptr_id++] = right_child;
                    k = true;
                    curr_ptr_id++;
                    new_node -> count++;
                } else {
                    new_node -> key[i] = curr_node -> key[key_idx++];
                    new_node -> ptr[new_ptr_id++].store((Node<K,V>* ) unset_mark((uintptr_t) curr_node -> ptr[curr_ptr_id++].load(std::memory_order_seq_cst)));
                }
            }
            if(!k) {
                new_node -> key[new_node -> count] = x;
                new_node -> ptr[new_node -> count] = left_child;
                new_node -> count++;
                new_node -> ptr[new_node -> count] = right_child;
            }
            Node<K, V> *t = curr_node;
            if (prev_node == nullptr) {
                if (!root.compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst,
                                                  std::memory_order_seq_cst)) {
                    delete new_node;
                    return nullptr;
                }
            } else {
                if (!prev_node->ptr[prev_idx].compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst,
                                                                      std::memory_order_seq_cst)) {
                    delete new_node;
                    return nullptr;
                }
            }
            return new_node;
        } else if (curr_child->count < MIN)  // Merge the current child
        {
            int64_t ls_idx = curr_idx - 1; // Left_Sibling Index
            int64_t rs_idx = curr_idx + 1; // Right Sibling Index
            if(ls_idx >= 0)        // Left Sibling Exist
            {
                Node<K,V>* left_child = (Node<K,V>*) unset_mark((uintptr_t )curr_node -> ptr[ls_idx].load(std::memory_order_seq_cst));
                Node<K,V>* temp = nullptr;
                left_child -> mark();
                left_child -> stabilize();
                Node<K, V>* res = merge(left_child, curr_child, curr_node, ls_idx, prev_node, prev_idx);
                return res;
            } else if (rs_idx <= curr_node->count) {
                Node<K, V> *right_child = (Node<K, V> *) unset_mark((uintptr_t) curr_node->ptr[rs_idx].load(std::memory_order_seq_cst));
                Node<K, V> *temp = nullptr;
                right_child->mark();
                right_child->stabilize();
                Node<K, V> *res = merge(curr_child, right_child, curr_node, curr_idx, prev_node, prev_idx);
                return res;
            }
        } else {      // Replace the current child
            curr_child -> create_new_leaf();
            Node<K,V>* new_node = new internal_node<K,V> ();
            new_node -> count.store(curr_node -> count);
            int64_t i;
            for(i = 0; i < curr_node -> count; i++)
            {
                new_node -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) curr_node -> ptr[i].load(std::memory_order_seq_cst)));
                new_node -> key[i] = curr_node -> key[i];
            }
            new_node -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) curr_node -> ptr[i].load(std::memory_order_seq_cst)));
            new_node -> ptr[curr_idx].store(curr_child -> new_next.load(std::memory_order_seq_cst));
            if (prev_node == nullptr) {
                if (!root.compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst,
                                                  std::memory_order_seq_cst))
                    return nullptr;
                else {
                    return new_node;
                }
            } else {
                if (!prev_node->ptr[prev_idx].compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst,
                                                                      std::memory_order_seq_cst))
                    return nullptr;
                else {
                    return new_node;
                }
            }
        }
    }
    else{
        if(curr_child -> count >= MAX)
        {
            curr_child -> mark();
            Node<K,V>* right_child = new internal_node<K,V> ();
            Node<K,V>* left_child = new internal_node<K,V> ();
            curr_child -> split_internal(left_child, right_child);
            Node<K,V>* new_node = new internal_node<K,V> ();
            int64_t x = curr_child -> key[left_child -> count];
            new_node -> count.store(curr_node -> count, std::memory_order_seq_cst);
            bool k = false;
            K key_idx = 0;
            int64_t curr_ptr_id = 0;
            int64_t new_ptr_id = 0;
            for (int64_t i = 0; i < new_node -> count; ++i) {
                if (!k && curr_node->key[key_idx] > x) {
                    new_node -> key[i] = x;
                    new_node -> ptr[new_ptr_id++] = left_child;
                    new_node -> ptr[new_ptr_id++] = right_child;
                    k = true;
                    curr_ptr_id++;
                    new_node -> count++;
                } else {
                    new_node -> key[i] = curr_node -> key[key_idx++];
                    new_node -> ptr[new_ptr_id++].store((Node<K,V>* ) unset_mark((uintptr_t) curr_node->ptr[curr_ptr_id++].load(std::memory_order_seq_cst)));
                }
            }
            if (!k) {
                new_node->key[new_node->count] = x;
                new_node->ptr[new_node->count] = left_child;
                new_node->count++;
                new_node->ptr[new_node->count] = right_child;
            }
            if (prev_node == nullptr) {
                if (!root.compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst)) {
                    delete new_node;
                    delete left_child;
                    delete right_child;
                    return nullptr;
                }
            }
            else {
                if (!prev_node->ptr[prev_idx].compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst)) {
                    delete new_node;
                    delete left_child;
                    delete right_child;
                    return nullptr;
                }
            }
            return new_node;
        }
        else
        {
            curr_node -> mark();
            curr_child -> mark();
            int64_t pc_idx = curr_idx - 1;
            int64_t nc_idx = curr_idx + 1;
            if(pc_idx >= 0)  //Left Child exist
            {
                Node<K,V>* left_child = (Node<K,V>*) unset_mark((uintptr_t)curr_node -> ptr[pc_idx].load(std::memory_order_seq_cst));
                left_child -> mark();
                bool res = merge(left_child, curr_child, curr_node, pc_idx, prev_node, prev_idx);
                return (Node<K,V>*) nullptr;
            }
            else if(nc_idx <= curr_node -> count )
            {
                Node<K,V>* right_child = (Node<K,V>*) unset_mark((uintptr_t )curr_node -> ptr[nc_idx].load(std::memory_order_seq_cst));
                right_child -> mark();
                bool res = merge(curr_child, right_child, curr_node, curr_idx, prev_node, prev_idx);
                return (Node<K,V>*) nullptr;
            }
        }
    }
}

template<typename K, typename V>
std::vector<std::pair<K,V>> BPTree<K,V>::range_query(int64_t begin, int64_t last)
{
//	int64_t curr_ts = global_rq_timestamp.fetch_add(1);
    int64_t curr_ts = version_tracker.add_timestamp() -> ts;
    std::vector<std::pair<K,V>> res;
    Node<K,V>* curr_node = (Node<K,V>*) unset_mark((uintptr_t) root.load(std::memory_order_seq_cst));
    if(curr_node == nullptr)
        return res;
    while(!curr_node -> is_leaf)
    {
        int64_t low = 0, high = curr_node->count - 1;
        while (low < high) {
            int64_t mid = low + (high - low) / 2;
            if (begin >= curr_node->key[mid]) {
                low = mid + 1;
            } else high = mid;
        }
        if (curr_node->key[low] <= begin) {
            curr_node = (Node<K,V>*) unset_mark((uintptr_t) curr_node ->ptr[curr_node->count].load(std::memory_order_seq_cst));
        } else {
            curr_node = (Node<K,V>*) unset_mark((uintptr_t) curr_node->ptr[low].load(std::memory_order_seq_cst));
        }
    }
    Node<K,V>* prev_node = nullptr;
    while(curr_node != nullptr)
    {
        if((curr_node -> new_next.load(std::memory_order_seq_cst) == nullptr))
        {
            if(curr_node -> data_array_list.head->next.load(std::memory_order_seq_cst)->key <= last) {
                curr_node -> data_array_list.range_query(begin, last, curr_ts, res);
                prev_node = curr_node;
                curr_node = curr_node->next;
            }
            else
                break;
        }
        else if(curr_node -> new_next.load(std::memory_order_seq_cst) != nullptr)
        {
            Node<K,V>* curr_node_new_next = curr_node -> new_next.load(std::memory_order_seq_cst);
            curr_node = curr_node_new_next;
            if(prev_node != nullptr)
            {
                prev_node -> next = curr_node;
            }
        }
    }
}

#endif //LF_B_TREEV2_BPTREE_H
