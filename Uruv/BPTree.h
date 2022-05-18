
#ifndef LF_B_TREEV2_BPTREE_H
#define LF_B_TREEV2_BPTREE_H

#include "Leaf_node.h"
#include "Internal_node.h"

template<typename K, typename V>
class BPTree{
public:
    int init_max = MAX;
    int init_min = MIN;
    std::atomic<Node<K,V>*> root;
    std::atomic<int64_t> size;
    std::vector<std::pair<K,V>>collect();
    std::vector<std::pair<K,V>> range_query(int64_t low, int64_t high);
    Node<K,V>* do_helping(internal_node<K,V>*, int64_t, Node<K,V>*);
    V search(K key);
    Node<K,V>* merge(Node<K,V>* left_child, Node<K,V>* right_child, Node<K,V>* curr_node, int64_t idx, internal_node<K,V>* prev_node, int64_t prev_idx);
    BPTree<K,V> (int threads)
    {
        root.store(new leaf_node<K,V>(MIN, MAX));
        for(int i = 0; i < threads; i++)
            stateArray.push_back(nullptr);
        NUM_THREADS = threads;
        size = 0;
    }
    bool remove(K, int tid, int phase);
    bool insert(K, V, int tid, int phase);
    void remove(K, int);
    void insert(K, V, int);
    Node<K,V>* remove_parent(Node<K,V>*, Node<K,V>*, int64_t);
    Node<K,V>* remove_parent(Node<K,V>*, Node<K,V>*, Node<K,V>* ,int64_t);
    void init_thread(int);
    std::pair<Node<K,V>*, int> search_child(K, Node<K,V>*);
    std::pair<Node<K,V>*, Node<K,V>*> balance_leaf(Node<K,V>*, leaf_node<K,V>*, internal_node<K,V>*, int prev_idx, int&, int);
    bool set_help_idx(Node<K,V>* curr_node, int curr_idx);
};

template<typename K, typename V>
std::pair<Node<K,V>*, Node<K,V>*> BPTree<K,V>::balance_leaf(Node<K, V>* curr_node, leaf_node<K, V>* curr_child, internal_node<K,V>* prev_node, int prev_idx, int &curr_idx, int key) {
    if((int64_t) curr_child -> count.load(std::memory_order_seq_cst) >= curr_child->max) //Split the Current Child
    {
        curr_child -> split_leaf();
        internal_node<K,V>* new_node = new internal_node<K,V>(curr_node->min, curr_node->max);
        leaf_node<K,V>* left_child = curr_child -> new_next.load(std::memory_order_seq_cst);
        leaf_node<K,V>* right_child = left_child -> next;
        int64_t x = right_child -> data_array_list.head -> next.load(std::memory_order_seq_cst) -> key;
        bool k = false;
        int64_t key_idx = 0;
        int64_t curr_ptr_id = 0;
        int64_t new_ptr_id = 0;
        new_node -> count.store(curr_node -> count.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
        for (int64_t i = 0; i < new_node -> count.load(std::memory_order_seq_cst); ++i) {
            if (!k && ((internal_node<K,V>*)curr_node) -> key[key_idx] > x) {
                new_node -> key[i] = x;
                new_node -> ptr[new_ptr_id++].store(left_child, std::memory_order_seq_cst);
                new_node -> ptr[new_ptr_id++].store(right_child, std::memory_order_seq_cst);
                k = true;
                curr_ptr_id++;
                new_node -> count++;
            } else {
                new_node -> key[i] = ((internal_node<K,V>*)curr_node) -> key[key_idx++];
                new_node -> ptr[new_ptr_id++].store((Node<K,V>* ) unset_mark((uintptr_t) ((internal_node<K,V>*)curr_node) -> ptr[curr_ptr_id++].load(std::memory_order_seq_cst)));
            }
        }
        if(!k) {
            int new_node_count = new_node -> count.load(std::memory_order_seq_cst);
            new_node -> key[new_node_count] = x;
            new_node -> ptr[new_node_count].store(left_child, std::memory_order_seq_cst);
            new_node -> count++;
            new_node -> ptr[new_node_count + 1].store(right_child, std::memory_order_seq_cst);
        }
        if (prev_node == nullptr) {
            if (!root.compare_exchange_strong(curr_node, new_node, std:: memory_order_seq_cst, std::memory_order_seq_cst)) {
                return std::make_pair(nullptr, nullptr);
            }
        }
        else {
            if (!prev_node -> ptr[prev_idx].compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
                return std::make_pair(nullptr, nullptr);
            }
        }
        if(key < x)
            return std::make_pair(new_node, left_child);
        else {
            curr_idx++;
            return std::make_pair(new_node, right_child);
        }
    }
    else if((int64_t)curr_child -> count.load(std::memory_order_seq_cst) < curr_child->min)  // Merge the current child
    {
        int64_t ls_idx = curr_idx - 1; // Left_Sibling Index
        int64_t rs_idx = curr_idx + 1; // Right Sibling Index
        Node<K,V>* res;
        if(ls_idx >= 0)        // Left Sibling Exist
        {
            leaf_node<K,V>* left_child = (leaf_node<K,V>*) unset_mark((uintptr_t) ((internal_node<K,V>*) curr_node) -> ptr[ls_idx].load(std::memory_order_seq_cst));
            left_child -> mark();
            ((leaf_node<K,V>*)left_child) -> stabilize();
            res = merge(left_child, curr_child, curr_node, ls_idx, prev_node, prev_idx);
        }
        else if(rs_idx <= curr_node -> count.load(std::memory_order_seq_cst))
        {
            leaf_node<K,V>* right_child = (leaf_node<K,V>*) unset_mark((uintptr_t )((internal_node<K,V>*) curr_node) -> ptr[rs_idx].load(std::memory_order_seq_cst));
            right_child -> mark();
            ((leaf_node<K,V>*)right_child) -> stabilize();
            res = merge(curr_child, right_child, curr_node, curr_idx, prev_node, prev_idx);
        }
        if(!res) {
            return std::make_pair(nullptr, nullptr);
        }
        else {
            std::pair<Node<K,V>*, int> res1 = search_child(key, res);
            curr_idx = res1.second;
            return std::make_pair(res, res1.first);
        }
    }
    else{      // Replace the current child
        curr_child -> create_new_leaf();
        internal_node<K,V>* new_node = new internal_node<K,V> (curr_node->min, curr_node->max);
        new_node -> count.store(curr_node -> count.load(std::memory_order_seq_cst));
        int64_t i;
        for(i = 0; i < curr_node -> count.load(std::memory_order_seq_cst); i++)
        {
            new_node -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) ((internal_node<K,V>*)curr_node) -> ptr[i].load(std::memory_order_seq_cst)));
            new_node -> key[i] = ((internal_node<K,V>*)curr_node) -> key[i];
        }
        new_node -> ptr[i].store((Node<K,V>*) unset_mark((uintptr_t) ((internal_node<K,V>*)curr_node) -> ptr[i].load(std::memory_order_seq_cst)));
        new_node -> ptr[curr_idx].store(curr_child -> new_next.load(std::memory_order_seq_cst));
        if(prev_node == nullptr)
        {
            if(!root.compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
                return std::make_pair(nullptr, nullptr);
            }
            else {
                return std::make_pair(new_node, curr_child -> new_next.load(std::memory_order_seq_cst));
            }
        }
        else
        {
            if(!prev_node -> ptr[prev_idx].compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
                return std::make_pair(nullptr, nullptr);
            }
            else {
                return std::make_pair(new_node, curr_child -> new_next.load(std::memory_order_seq_cst));
            }
        }
    }
}




template<typename K, typename V>
bool BPTree<K,V>::set_help_idx(Node<K, V> *curr_node, int curr_idx) {
    if(((internal_node<K,V>*)curr_node) -> help_idx.load(std::memory_order_seq_cst) == -1) // Help_idx is not set
    {
        int64_t temp = -1;
        if(!((internal_node<K,V>*)curr_node) -> help_idx.compare_exchange_strong(temp, curr_idx, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
            return false;
        }
        return true;
    }
    else {
        return false;
    }

}

template<typename K, typename V>
std::pair<Node<K,V>*, int> BPTree<K,V>::search_child(K key, Node<K,V>* curr_node)
{
    int64_t low = 0, high = curr_node->count.load(std::memory_order_seq_cst) - 1;
    int curr_idx = -1;
    while (low < high) {
        int64_t mid = low + (high - low) / 2;
        if (key >= ((internal_node<K,V>*)curr_node)->key[mid]) {
            low = mid + 1;
        } else high = mid;
    }
    if (((internal_node<K,V>*)curr_node)->key[low] <= key) {
        return std::pair((Node<K,V>*) unset_mark((uintptr_t) ((internal_node<K,V>*)curr_node) ->ptr[curr_node->count.load(std::memory_order_seq_cst)].load(std::memory_order_seq_cst)), curr_node->count.load(std::memory_order_seq_cst));
    } else {
        return std::pair((Node<K,V>*) unset_mark((uintptr_t) ((internal_node<K,V>*)curr_node)->ptr[low].load(std::memory_order_seq_cst)), low);
    }
}


template<typename K, typename V>
V BPTree<K,V>:: search(K key){
    Node<K,V>* curr_node = (Node<K,V>*) unset_mark((uintptr_t) root.load(std::memory_order_seq_cst));
    if(curr_node == nullptr)
        return -1;
    while(!curr_node -> is_leaf)
    {
        curr_node = search_child(key, curr_node).first;
    }
    return ((leaf_node<K,V>*)curr_node) -> data_array_list.search(key);
}

template<typename K, typename V>
void BPTree<K,V>::init_thread(int tid) // Initialize the helprecord for each thread
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
    if(curr_node -> is_leaf)
    {
        if(curr_node -> status > 0)
        {
            curr_node -> mark();
            ((leaf_node<K,V>*)curr_node) -> stabilize();
            if(curr_node -> count.load(std::memory_order_seq_cst) >= curr_node->max)
            {
                ((leaf_node<K,V>*)curr_node) -> split_leaf();
                internal_node<K,V>* new_node = new internal_node<K,V> (curr_node->min*1 , curr_node->max*1 );
                new_node -> count.store(1, std::memory_order_seq_cst);
                leaf_node<K,V>* left_child = ((leaf_node<K,V>*)curr_node) -> new_next.load(std::memory_order_seq_cst);
                leaf_node<K,V>* right_child = ((leaf_node<K,V>*)left_child) -> next;
                new_node -> ptr[0].store(left_child, std::memory_order_seq_cst);
                new_node -> ptr[1].store(right_child, std::memory_order_seq_cst);
                new_node -> key[0] = right_child -> data_array_list.head -> next.load(std::memory_order_seq_cst) -> key;
                if(!root.compare_exchange_strong(curr_node, (Node<K,V>*)new_node, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
                    goto retry;
                }
                curr_node = new_node;
            }
            else
            {
                ((leaf_node<K,V>*)curr_node) -> create_new_leaf();
                if(!root.compare_exchange_strong(curr_node, ((leaf_node<K,V>*)curr_node) -> new_next.load(std::memory_order_seq_cst), std::memory_order_seq_cst, std::memory_order_seq_cst)) {
                    goto retry;
                }
                curr_node = ((leaf_node<K,V>*)curr_node) -> new_next.load(std::memory_order_seq_cst);
            }
        }
    }
    else if(curr_node -> count.load(std::memory_order_seq_cst) >= curr_node -> max) // if root is an Internal node and it's full
    {
        curr_node -> mark();
        internal_node<K,V>* left_child = new internal_node<K,V> (curr_node->min, curr_node->max);
        internal_node<K,V>* right_child = new internal_node<K,V> (curr_node->min, curr_node->max);
        ((internal_node<K,V>*)curr_node) -> split_internal (left_child, right_child);
        internal_node<K,V>* new_node = new internal_node<K,V> (curr_node -> min*1 , curr_node -> max*1 );
        new_node -> ptr[0].store(left_child, std::memory_order_seq_cst);
        new_node -> ptr[1].store(right_child, std::memory_order_seq_cst);
        new_node -> key[0] = ((internal_node<K,V>*)curr_node) -> key[left_child -> count.load(std::memory_order_seq_cst)];
        new_node -> count.store(1, std::memory_order_seq_cst);
        if(!root.compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst))
        {
            goto retry;
        }
        curr_node = new_node;
    }
    internal_node<K,V>* prev_node = nullptr;
    int64_t prev_idx = -1;
    while(!curr_node -> is_leaf)
    {
        if(((internal_node<K,V>*)curr_node) -> help_idx.load(std::memory_order_seq_cst) != -1)
        {
            Node<K,V>* res = do_helping(prev_node, prev_idx, curr_node);
            if(!res || res->count == res->max) {
                goto retry;
            }
            else
                curr_node = res;
        }
        Node<K, V>* curr_child = nullptr;
        int curr_idx = -1;
        std::pair<Node<K,V>*, int> res = search_child(key, curr_node);
        curr_child = res.first;
        curr_idx = res.second;
        if(curr_child -> is_leaf && curr_child -> status.load(std::memory_order_seq_cst) > 0)
        {
            curr_node -> mark();
            ((leaf_node<K,V>*)curr_child) -> stabilize();
            if(!set_help_idx(curr_node, curr_idx))
                goto retry;
            std::pair<Node<K,V>*, Node<K,V>*> res = balance_leaf(curr_node, (leaf_node<K,V>*)curr_child, (internal_node<K,V>*)prev_node, prev_idx, curr_idx, key);
            curr_node = res.first;
            curr_child = res.second;
            if(curr_node == nullptr || curr_child == nullptr)
                goto retry;
        }
        else if(!curr_child -> is_leaf && curr_child -> count.load(std::memory_order_seq_cst) >= curr_child->max) // if curr_child is not leaf
        {
            curr_node -> mark();
            if(!set_help_idx(curr_node, curr_idx))
                goto retry;
            curr_child -> mark();
            internal_node<K,V>* right_child = new internal_node<K,V> (curr_child->min, curr_child->max);
            internal_node<K,V>* left_child = new internal_node<K,V> (curr_child->min, curr_child->max);
            ((internal_node<K,V>*)curr_child) -> split_internal(left_child, right_child);
            internal_node<K,V>* new_node = new internal_node<K,V> (curr_node->min, curr_node->max);
            int64_t x = ((internal_node<K,V>*)curr_child) -> key[left_child -> count.load(std::memory_order_seq_cst)];
            new_node -> count.store(curr_node -> count.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
            bool k = false;
            K key_idx = 0;
            int64_t curr_ptr_id = 0;
            int64_t new_ptr_id = 0;
            for (int64_t i = 0; i < new_node -> count.load(std::memory_order_seq_cst); ++i) {
                if (!k && ((internal_node<K,V>*)curr_node)->key[key_idx] > x) {
                    new_node -> key[i] = x;
                    new_node -> ptr[new_ptr_id++] = left_child;
                    new_node -> ptr[new_ptr_id++] = right_child;
                    k = true;
                    curr_ptr_id++;
                    new_node -> count++;
                } else {
                    new_node -> key[i] = ((internal_node<K,V>*)curr_node) -> key[key_idx++];
                    new_node -> ptr[new_ptr_id++].store((Node<K,V>* ) unset_mark((uintptr_t) ((internal_node<K,V>*)curr_node)->ptr[curr_ptr_id++].load(std::memory_order_seq_cst)));
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
                    goto retry;
                }
            }
            else {
                if (!prev_node->ptr[prev_idx].compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst)) {
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
        prev_node = (internal_node<K,V>*)curr_node;
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
    if(curr_node == nullptr){
        return true;
    }
    if(curr_node -> is_leaf)
    {
        if(curr_node -> status == 0) {
            int64_t result;
            if(phase == -1)
                result = ((leaf_node<K,V>*)curr_node) -> delete_leaf(key);
            else
                result = ((leaf_node<K,V>*)curr_node) -> delete_leaf(key, tid, phase);
            if (result == 1) {
                size--;
                return true;
            } else if (result == 0) {
                goto retry;
            } else {
                return true;
            }
        }
        else
        {
            curr_node -> mark();
            ((leaf_node<K,V>*)curr_node) -> stabilize();
            if(curr_node -> count.load(std::memory_order_seq_cst) >= curr_node->max)
            {
                ((leaf_node<K,V>*)curr_node) -> split_leaf();
                internal_node<K,V>* new_node = new internal_node<K,V> (curr_node -> min*1  , curr_node -> max*1 );
                new_node -> count.store(1, std::memory_order_seq_cst);
                leaf_node<K,V>* left_child = ((leaf_node<K,V>*)curr_node) -> new_next.load(std::memory_order_seq_cst);
                leaf_node<K,V>* right_child = left_child -> next;
                new_node -> ptr[0] = left_child;
                new_node -> ptr[1] = right_child;
                new_node -> key[0] = right_child -> data_array_list.head -> next.load(std::memory_order_seq_cst) -> key;
                root.compare_exchange_strong(curr_node, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst);
                curr_node = new_node;
                goto retry;
            }
            else
            {
                ((leaf_node<K,V>*)curr_node) -> create_new_leaf();
                root.compare_exchange_strong(curr_node, ((leaf_node<K,V>*)curr_node) -> new_next.load(std::memory_order_seq_cst), std::memory_order_seq_cst, std::memory_order_seq_cst);
                curr_node = ((leaf_node<K,V>*)curr_node) -> new_next.load(std::memory_order_seq_cst);
                goto retry;
            }
        }
    }
    internal_node<K,V>* prev_node = nullptr;
    Node<K,V>* old_curr_child = nullptr;
    int64_t prev_idx = -1;
    while(!curr_node -> is_leaf)
    {
        if(((internal_node<K,V>*)curr_node) -> help_idx.load(std::memory_order_seq_cst) != -1)
        {
            Node<K,V>* res = do_helping(prev_node, prev_idx, curr_node);
            if(!res) {
                goto retry;
            }
            else
                curr_node = res;
        }
        Node<K, V>* curr_child = nullptr;
        int curr_idx = -1;
        std::pair<Node<K,V>*, int64_t> res = search_child(key, curr_node);
        curr_child = res.first;
        curr_idx = res.second;
        if(curr_child -> is_leaf && curr_child -> status > 0)
        {
            curr_node -> mark();
            curr_child -> mark();
            ((leaf_node<K,V>*)curr_child) -> stabilize();
            if(!set_help_idx(curr_node, curr_idx))
                goto retry;
            std::pair<Node<K,V>*, Node<K,V>*> res = balance_leaf(curr_node, (leaf_node<K,V>*)curr_child, (internal_node<K,V>*)prev_node, prev_idx, curr_idx, key);
            curr_node = res.first;
            curr_child = res.second;
            if(curr_node == nullptr || curr_child == nullptr)
                goto retry;
        }
        else if(!curr_child -> is_leaf && curr_child -> count < curr_child -> min){
            curr_child -> mark();
            if(!set_help_idx(curr_node, curr_idx))
                goto retry;
            curr_child -> mark();
            int64_t pc_idx = curr_idx - 1;
            int64_t nc_idx = curr_idx + 1;
            if(pc_idx >= 0)  //Left Child exist
            {
                Node<K,V>* left_child = (Node<K,V>*) unset_mark((uintptr_t)((internal_node<K,V>*)curr_node) -> ptr[pc_idx].load(std::memory_order_seq_cst));
                left_child -> mark();
                bool res = merge(left_child, curr_child, (internal_node<K,V>*)curr_node, pc_idx, prev_node, prev_idx);
                if(res)
                    continue;
                else {
                    goto retry;
                }
            }
            else if(nc_idx <= curr_node -> count )
            {
                Node<K,V>* right_child = (Node<K,V>*) unset_mark((uintptr_t )((internal_node<K,V>*)curr_node) -> ptr[nc_idx].load(std::memory_order_seq_cst));
                right_child -> mark();
                bool res = merge(curr_child, right_child, (internal_node<K,V>*)curr_node, curr_idx, prev_node, prev_idx);
                if(res)
                    continue;
                else {
                    goto retry;
                }
            }
        }
        prev_node = (internal_node<K,V>*)curr_node;
        curr_node = curr_child;
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
Node<K,V>* BPTree<K,V>::merge(Node<K,V>* left_child, Node<K,V>* right_child, Node<K,V>* curr_node, int64_t idx, internal_node<K,V>* prev_node, int64_t prev_idx)
{
    if(left_child -> is_leaf) {
        bool merged = ((leaf_node<K,V>*)left_child) -> merge_leaf((leaf_node<K,V>*)right_child);
        if (merged) {
            Node<K,V>* new_node = remove_parent(curr_node, ((leaf_node<K,V>*)left_child) -> new_next, idx);
            if (new_node -> count == 0 && prev_node == nullptr) {
                root.compare_exchange_strong(curr_node, (Node<K,V>*) ((leaf_node<K,V>*)left_child) -> new_next, std::memory_order_seq_cst, std::memory_order_seq_cst);
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
            Node<K,V>* new_node = remove_parent(curr_node, ((leaf_node<K,V>*)left_child) -> new_next.load(std::memory_order_seq_cst), ((leaf_node<K,V>*)left_child) -> new_next.load(std::memory_order_seq_cst) -> next, idx);
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
        internal_node<K,V>* new_node = ((internal_node<K,V>*)left_child) -> merge_internal((internal_node<K,V>*)curr_node, (internal_node<K,V>*)right_child, idx);
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
    internal_node<K,V>* new_node = new internal_node<K,V> (curr_node->min, curr_node->max);
    new_node -> count = curr_node -> count - 1;
    int64_t i = 0;
    for (; i < idx; i++) {
        new_node -> key[i] = ((internal_node<K,V>*)curr_node)->key[i];
        new_node -> ptr[i].store((Node<K,V>* ) unset_mark((uintptr_t) ((internal_node<K,V>*)curr_node)->ptr[i].load(std::memory_order_seq_cst)));
    }
    new_node -> ptr[i].store(left_child);
    for (; i < new_node -> count; i++) {
        new_node->key[i] = ((internal_node<K,V>*)curr_node)->key[i + 1];
        new_node->ptr[i + 1].store((Node<K,V>* ) unset_mark((uintptr_t) ((internal_node<K,V>*)curr_node)->ptr[i + 2].load(std::memory_order_seq_cst)));
    }
    return new_node;
}

template<typename K, typename V>
Node<K,V>* BPTree<K,V>::remove_parent(Node<K,V>* curr_node, Node<K,V>* left_child , Node<K,V>* right_child, int64_t idx) {
    internal_node<K,V>* new_node = new internal_node<K,V> (curr_node -> min, curr_node -> max);
    new_node -> count.store(curr_node -> count);
    int64_t i = 0;
    for (; i < idx; i++) {
        new_node -> key[i] = ((internal_node<K,V>*)curr_node) -> key[i];
        new_node -> ptr[i].store((Node<K,V>* ) unset_mark((uintptr_t) ((internal_node<K,V>*)curr_node) -> ptr[i].load(std::memory_order_seq_cst)));
    }
    new_node -> ptr[i].store(left_child, std::memory_order_seq_cst);
    new_node -> key[i] = ((leaf_node<K,V>*)right_child) -> data_array_list.head -> next.load(std::memory_order_seq_cst) -> key;
    i++;
    new_node -> ptr[i].store(right_child, std::memory_order_seq_cst);
    for(; i < new_node -> count; i++)
    {
        new_node -> key[i] = ((internal_node<K,V>*)curr_node) -> key[i];
        new_node -> ptr[i + 1].store((Node<K,V>* ) unset_mark((uintptr_t) ((internal_node<K,V>*)curr_node) -> ptr[i + 1].load(std::memory_order_seq_cst)));
    }
    return new_node;
}

template<typename K, typename V>
Node<K, V>* BPTree<K,V> :: do_helping(internal_node<K, V>* prev_node, int64_t prev_idx, Node<K, V>* curr_node) {
    int curr_idx = ((internal_node<K,V>*)curr_node) -> help_idx.load(std::memory_order_seq_cst);
    Node<K,V>* curr_child = (Node<K,V>*) unset_mark((uintptr_t) ((internal_node<K,V>*)curr_node) -> ptr[curr_idx].load(std::memory_order_seq_cst));
    if(curr_child -> is_leaf) {
        std::pair<Node<K,V>*, Node<K,V>*> res = balance_leaf(curr_node, (leaf_node<K,V>*)curr_child, prev_node, prev_idx, curr_idx, 0);
        return res.first;
    }
    else{
        if(curr_child -> count >= curr_child-> max)
        {
            curr_child -> mark();
            Node<K,V>* right_child = new internal_node<K,V> (curr_child->min, curr_child->max);
            Node<K,V>* left_child = new internal_node<K,V> (curr_child->min, curr_child->max);
            ((internal_node<K,V>*)curr_child) -> split_internal((internal_node<K,V>*)left_child, (internal_node<K,V>*)right_child);
            internal_node<K,V>* new_node = new internal_node<K,V> (curr_node->min, curr_node->max);
            int64_t x = ((internal_node<K,V>*)curr_child) -> key[left_child -> count];
            new_node -> count.store(curr_node -> count, std::memory_order_seq_cst);
            bool k = false;
            K key_idx = 0;
            int64_t curr_ptr_id = 0;
            int64_t new_ptr_id = 0;
            for (int64_t i = 0; i < new_node -> count; ++i) {
                if (!k && ((internal_node<K,V>*)curr_node)->key[key_idx] > x) {
                    new_node -> key[i] = x;
                    new_node -> ptr[new_ptr_id++] = left_child;
                    new_node -> ptr[new_ptr_id++] = right_child;
                    k = true;
                    curr_ptr_id++;
                    new_node -> count++;
                } else {
                    new_node -> key[i] = ((internal_node<K,V>*)curr_node) -> key[key_idx++];
                    new_node -> ptr[new_ptr_id++].store((Node<K,V>* ) unset_mark((uintptr_t) ((internal_node<K,V>*)curr_node)->ptr[curr_ptr_id++].load(std::memory_order_seq_cst)));
                }
            }
            if (!k) {
                new_node->key[new_node->count] = x;
                new_node->ptr[new_node->count] = left_child;
                new_node->count++;
                new_node->ptr[new_node->count] = right_child;
            }
            if (prev_node == nullptr) {
                if (!root.compare_exchange_strong(curr_node, (Node<K,V>*)new_node, std::memory_order_seq_cst)) {
                    return nullptr;
                }
            }
            else {
                if (!prev_node->ptr[prev_idx].compare_exchange_strong(curr_node, (Node<K,V>*)new_node, std::memory_order_seq_cst)) {
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
                Node<K,V>* left_child = (Node<K,V>*) unset_mark((uintptr_t)((internal_node<K,V>*)((internal_node<K,V>*)curr_node)) -> ptr[pc_idx].load(std::memory_order_seq_cst));
                left_child -> mark();
                merge(left_child, curr_child, ((internal_node<K,V>*)curr_node), pc_idx, prev_node, prev_idx);
                return (Node<K,V>*) nullptr;
            }
            else if(nc_idx <= curr_node -> count )
            {
                Node<K,V>* right_child = (Node<K,V>*) unset_mark((uintptr_t )((internal_node<K,V>*)curr_node) -> ptr[nc_idx].load(std::memory_order_seq_cst));
                right_child -> mark();
                merge(curr_child, right_child, ((internal_node<K,V>*)curr_node), curr_idx, prev_node, prev_idx);
                return (Node<K,V>*) nullptr;
            }
        }
    }
}

template<typename K, typename V>
std::vector<std::pair<K,V>> BPTree<K,V>::range_query(int64_t begin, int64_t last)
{
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
            if (begin >= ((internal_node<K,V>*)curr_node)->key[mid]) {
                low = mid + 1;
            } else high = mid;
        }
        if (((internal_node<K,V>*)curr_node)->key[low] <= begin) {
            curr_node = (Node<K,V>*) unset_mark((uintptr_t) ((internal_node<K,V>*)curr_node) ->ptr[curr_node->count].load(std::memory_order_seq_cst));
        } else {
            curr_node = (Node<K,V>*) unset_mark((uintptr_t) ((internal_node<K,V>*)curr_node)->ptr[low].load(std::memory_order_seq_cst));
        }
    }
    leaf_node<K,V>* prev_node = nullptr;
    while(curr_node != nullptr)
    {
        if(((leaf_node<K,V>*)curr_node) -> new_next.load(std::memory_order_seq_cst) == nullptr)
        {
            if(((leaf_node<K,V>*)curr_node) -> data_array_list.head->next.load(std::memory_order_seq_cst)->key <= last) {
                ((leaf_node<K,V>*)curr_node) -> data_array_list.range_query(begin, last, curr_ts, res);
                prev_node = ((leaf_node<K,V>*)curr_node);
                curr_node = ((leaf_node<K,V>*)curr_node)->next;
            }
            else
                break;
        }
        else if(((leaf_node<K,V>*)curr_node) -> new_next.load(std::memory_order_seq_cst) != nullptr)
        {
            leaf_node<K,V>* curr_node_new_next = ((leaf_node<K,V>*)curr_node) -> new_next.load(std::memory_order_seq_cst);
            curr_node = curr_node_new_next;
            if(prev_node != nullptr)
            {
                prev_node -> next = ((leaf_node<K,V>*)curr_node);
            }
        }
    }
}

#endif //LF_B_TREEV2_BPTREE_H
