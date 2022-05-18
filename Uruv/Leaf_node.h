
#ifndef UNTITLED_LEAF_NODE_H
#define UNTITLED_LEAF_NODE_H

#include "Node.h"

template<typename K, typename V>
class leaf_node: public Node<int64_t, int64_t>{
public:
    std::atomic<int64_t> node_count;
    std::atomic<std::vector<std::pair<K,Vnode<V>*>>*> res;
    leaf_node<K,V>* next = nullptr;
    std::atomic<leaf_node<K,V>*> new_next = nullptr;
    Linked_List<K,V> data_array_list;
    void create_new_leaf();
    int64_t insert_leaf(K,V, int tid = -1, int phase = -1);
    void stabilize();
    void split_leaf();
    void mark();
    int64_t delete_leaf(K);
    int64_t delete_leaf(K key, int, int);
    bool merge_leaf(leaf_node<K,V>* right_child);
    leaf_node<K,V> (int min, int max)
    {
        this -> node_count = 0;
        this->min = min;
        this-> max = max;
        is_leaf = true;
        new_next = nullptr;
        status = 0;
        res.store(nullptr, std::memory_order_seq_cst);
    }
};

template<typename K, typename V>
void leaf_node<K,V>::mark(){
    if(status > 0)
        return;
    data_array_list.mark();
    if(status == 0){
        int64_t temp = 0;
        status.compare_exchange_strong(temp,temp + 1, std::memory_order_seq_cst, std::memory_order_seq_cst);
    }
}

template<typename K, typename V>
void leaf_node<K,V>:: stabilize()
{
    if(status > 1)
        return;
    std::vector<std::pair<K,Vnode<V>*>>* res1 = new std::vector<std::pair<K,Vnode<V>*>>();
    int64_t min_ts = version_tracker.remove_head() -> ts;
    data_array_list.collect(res1, min_ts);
    std::vector<std::pair<K,Vnode<V>*>>* curr_res = nullptr;
    res.compare_exchange_strong(curr_res, res1, std::memory_order_seq_cst, std::memory_order_seq_cst);
	count.store((int64_t)(*res.load(std::memory_order_seq_cst)).size(), std::memory_order_seq_cst);
	if(status == 1){
        int64_t temp = 1;
        status.compare_exchange_strong(temp,temp + 1, std::memory_order_seq_cst, std::memory_order_seq_cst);
    }
}

template<typename K, typename V>
int64_t leaf_node<K,V>::insert_leaf(K key, V value, int tid, int phase)
{
    while(true) {
        if (status == 0) {
            if(node_count.load(std::memory_order_seq_cst) >= max)
            {
                mark();
                stabilize();
                return 0;
            }
            node_count++;
            int result;
            if(phase == -1)
                result = data_array_list.insert(key,value);
            else
                result = data_array_list.insert(key,value, tid, phase);
            if( result == 1)
                return 1;
            else {
                node_count--;
                if(result == 0)
                    return -1;
                else if(result == -1)
                    return 0;
            }
        }
        else
            return 0;
    }
}

template<typename K, typename V>
int64_t leaf_node<K,V>::delete_leaf(K key, int tid, int phase) {
    State<K, V> *curr_state = stateArray[tid];
    ll_Node<K, V> *curr_node = curr_state->search_node.load(std::memory_order_seq_cst);
    if (curr_node == nullptr) {
        curr_state->finished = true;
        return -1;
    }
    ll_Node<K, V> *node;
    if (curr_node == dummy_node) {
        // TO DO to find can return null even if the node is frozen.
        node = data_array_list.find(key);
        ll_Node<K, V> *dummy_copy = dummy_node;
        curr_state->search_node.compare_exchange_strong(dummy_copy, node, std::memory_order_seq_cst,
                                                        std::memory_order_seq_cst);
    }
    node = curr_state->search_node.load(std::memory_order_seq_cst);
    if (curr_state -> vnode->ts != -1 || curr_state -> phase != phase){
        curr_state -> finished = true;
        return -1;
    }
    if(node == nullptr) {
        curr_state -> finished = true;
        return -1;
    }
    if(node -> key == key){
        if(data_array_list.read(node) == -1)
            return -1;
        int result = data_array_list.insert(key, -1, tid, phase);
        if(result == 0)
            return 1;
        else if(result == -1)
            return 0;
    }
    curr_state -> finished = true;
    return -1;
}

template<typename K, typename V>
int64_t leaf_node<K,V>::delete_leaf(K key) {
    ll_Node<K,V>* node = data_array_list.find(key);
    if(node == nullptr)
        return -1;
    if(node -> key == key){
		if(data_array_list.read(node) == -1)
			return -1;
        int result = data_array_list.insert(key, -1);
        if(result == 0)
            return 1;
        else if(result == -1)
            return 0;
    }
    return -1;
}

template<typename K, typename V>
void leaf_node<K,V> :: split_leaf()
{
    while(true) {
        if (new_next)
            return;
        leaf_node<K, V> *left_child = new leaf_node<K, V>(min, max);
        leaf_node<K, V> *right_child = new leaf_node<K, V>(min, max);
        std::vector<std::pair<K,Vnode<V>*>>* res1 = res.load(std::memory_order_seq_cst);
        left_child -> next = right_child;
        right_child -> next = next;
        int curr_node_count = count.load(std::memory_order_seq_cst);
        left_child -> node_count.store(curr_node_count / 2, std::memory_order_seq_cst);
        int left_count = curr_node_count / 2;
        right_child -> node_count.store(curr_node_count - left_count);
        int right_count = curr_node_count - left_count;
        int64_t j = 0;
        ll_Node<K,V> *curr_list_node = left_child -> data_array_list.head;
        ll_Node<K,V> *last_list_node = left_child -> data_array_list.head -> next.load(std::memory_order_seq_cst);
        for (int64_t i = 0; i < left_count; i++) {
            ll_Node<K,V> *new_node = new ll_Node<K,V>((*res1)[j].first, (*res1)[j].second, last_list_node);
            curr_list_node -> next.store(new_node);
            curr_list_node = new_node;
            j++;
        }
        curr_list_node = right_child -> data_array_list.head;
        last_list_node = right_child -> data_array_list.head -> next.load(std::memory_order_seq_cst);
        for (int64_t i = 0; i < right_count; i++) {
            ll_Node<K,V> *new_node = new ll_Node<K,V>((*res1)[j].first, (*res1)[j].second, last_list_node);
            curr_list_node -> next.store(new_node);
            curr_list_node = new_node;
            j++;
        }
        leaf_node<K,V> *temp = nullptr;
        if(new_next.compare_exchange_strong(temp, left_child, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
            return;
        }
    }
}

template<typename K, typename V>
bool leaf_node<K,V>::merge_leaf(leaf_node<K,V>* right_child) {
    std::vector<std::pair<K, Vnode<V>*>>* res_lc = res.load(std::memory_order_seq_cst);
    std::vector<std::pair<K, Vnode<V>*>>* res_rc = right_child -> res.load(std::memory_order_seq_cst);
    if(new_next.load(std::memory_order_seq_cst)) {
        if (count.load(std::memory_order_seq_cst) + right_child -> count.load(std::memory_order_seq_cst) < max)
            return true;
        else
            return false;
    }
    if(count.load(std::memory_order_seq_cst) + right_child -> count.load(std::memory_order_seq_cst) < max)  // Merging Left Child and right child
    {
        leaf_node<K,V>* merged_leaf = new leaf_node<K,V> (min,max);
        merged_leaf -> node_count.store(count.load(std::memory_order_seq_cst) + right_child -> count.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
        ll_Node<K, V> *curr_list_node = merged_leaf -> data_array_list.head;
        ll_Node<K, V> *last_list_node = merged_leaf -> data_array_list.head -> next.load(std::memory_order_seq_cst);
        int64_t i = 0;
        for (int64_t j = 0; j < count.load(std::memory_order_seq_cst); j++) {
            ll_Node<K, V> *new_node = new ll_Node<K,V>((*res_lc)[j].first, (*res_lc)[j].second, last_list_node);
            curr_list_node -> next.store(new_node);
            curr_list_node = new_node;
        }
        for (int64_t j = 0; j < right_child -> count.load(std::memory_order_seq_cst); j++) {
            ll_Node<K,V> *new_node = new ll_Node<K,V>((*res_rc)[j].first, (*res_rc)[j].second, last_list_node);
            curr_list_node -> next.store(new_node);
            curr_list_node = new_node;
        }
        merged_leaf -> next = right_child -> next;
        leaf_node<K,V>* temp = nullptr;
        new_next.compare_exchange_strong(temp, merged_leaf, std::memory_order_seq_cst, std::memory_order_seq_cst);
        return true;    // Merge Happens
    }
    else{  // Borrow from Sibling
        leaf_node<K,V> * left_merged = new leaf_node<K,V> (min, max);
        leaf_node<K,V> * right_merged = new leaf_node<K,V> (min, max);
        if(count.load(std::memory_order_seq_cst) < min) // Borrow from right child
        {
            left_merged -> node_count.store(count.load(std::memory_order_seq_cst) + 1);
            right_merged -> node_count.store(right_child -> count.load(std::memory_order_seq_cst) - 1);
            int64_t j = 0;
            int64_t i = 0;
            ll_Node<K,V> *curr_list_node = left_merged -> data_array_list.head;
            ll_Node<K,V> *last_list_node = left_merged -> data_array_list.head -> next.load(std::memory_order_seq_cst);
            for (; i < left_merged -> node_count - 1; i++) {
                ll_Node<K,V> *new_node = new ll_Node<K,V>((*res_lc)[j].first, (*res_lc)[j].second, last_list_node);
                curr_list_node -> next.store(new_node);
                curr_list_node = new_node;
                j++;
            }
            ll_Node<K,V> *new_node = new ll_Node<K,V>((*res_rc)[0].first, (*res_rc)[0].second, last_list_node);
            curr_list_node -> next.store(new_node);
            j = 1;
            curr_list_node = right_merged -> data_array_list.head;
            last_list_node = right_merged -> data_array_list.head -> next.load(std::memory_order_seq_cst);
            for (int64_t i = 0; i < right_merged -> node_count; i++) {
                ll_Node<K,V> *new_node = new ll_Node<K,V>((*res_rc)[j].first, (*res_rc)[j].second, last_list_node);
                curr_list_node -> next.store(new_node);
                curr_list_node = new_node;
                j++;
            }
        }
        else{ // Borrow from left sibling
            left_merged -> node_count.store(count.load(std::memory_order_seq_cst) - 1);
            right_merged -> node_count.store(right_child -> count.load(std::memory_order_seq_cst) + 1);
            int64_t j = 0;
            ll_Node<K,V> *curr_list_node = left_merged -> data_array_list.head;
            ll_Node<K,V> *last_list_node = left_merged -> data_array_list.head -> next.load(std::memory_order_seq_cst);
            for (int64_t i = 0; i < left_merged -> node_count; i++) {
                ll_Node<K,V> *new_node = new ll_Node<K,V>((*res_lc)[j].first, (*res_lc)[j].second, last_list_node);
                curr_list_node -> next.store(new_node);
                curr_list_node = new_node;
                j++;
            }
            curr_list_node = right_merged -> data_array_list.head;
            last_list_node = right_merged -> data_array_list.head -> next.load(std::memory_order_seq_cst);
            ll_Node<K,V> *new_node = new ll_Node<K,V>((*res_lc)[j].first, (*res_lc)[j].second,
                                                      last_list_node);
            curr_list_node -> next.store(new_node);
            curr_list_node = new_node;
            j = 0;
            for (int64_t i = 1; i < right_merged -> node_count; i++) {
                ll_Node<K,V> *new_node = new ll_Node<K,V>((*res_rc)[j].first, (*res_rc)[j].second,
                                                          last_list_node);
                curr_list_node -> next.store(new_node);
                curr_list_node = new_node;
                j++;
            }
        }
        left_merged -> next = right_merged;
        right_merged -> next = right_child -> next;
        leaf_node<K,V>* temp = nullptr;
        new_next.compare_exchange_strong(temp,left_merged, std::memory_order_seq_cst, std::memory_order_seq_cst);
        return false;  // Merge is not Happening, borrow is
    }
}

template<typename K, typename V>
void leaf_node<K,V>::create_new_leaf() {
    if(new_next){
        return;
    }
    std::vector<std::pair<K,Vnode<V>*>>* res1 = res.load(std::memory_order_seq_cst);
    leaf_node<K,V>* new_leaf = new leaf_node<K,V>(min, max);
    ll_Node<K,V> *curr_list_node = new_leaf -> data_array_list.head;
    ll_Node<K,V> *last_list_node = new_leaf -> data_array_list.head -> next.load(std::memory_order_seq_cst);
    for(int64_t i = 0; i < count.load(std::memory_order_seq_cst); i++)
    {
        ll_Node<K,V> *new_node = new ll_Node<K,V>((*res1)[i].first, (*res1)[i].second, last_list_node);
        curr_list_node -> next.store(new_node);
        curr_list_node = new_node;
    }
    new_leaf -> next = next;
    new_leaf -> node_count.store(count.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
    leaf_node<K,V>* temp = nullptr;
    new_next.compare_exchange_strong(temp, new_leaf, std::memory_order_seq_cst, std::memory_order_seq_cst);
    return;
}

#endif //UNTITLED_LEAF_NODE_H
