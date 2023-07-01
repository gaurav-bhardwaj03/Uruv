//
// Created by gaurav on 31/03/22.
//

#ifndef UNTITLED_UTIL_H
#define UNTITLED_UTIL_H


#include<atomic>
#include <random>
#include<vector>
#include<stdint.h>
#include "TrackerList/TrackerList.h"

const int64_t MAX = 32;
const int64_t MIN = 8;

std::atomic<int64_t> phase_counter = 0;
int NUM_THREADS;
int HELPING_DELAY = 1;
int MAX_FAILURE = 1000000000;
std::vector<int64_t> worst_case_resp_time;
thread_local int FAILURE = 0;
thread_local int cnt = 0;
thread_local int ttid;
bool expt_sleep = false;
thread_local std::uniform_int_distribution<int> dice(1, 10);
thread_local std::random_device rd;
thread_local std::mt19937 gen(rd());

template<typename V>
class Vnode{
public:
    V value;
    std::atomic<int64_t> ts;
    std::atomic<Vnode*> nextv;
    Vnode(){
        this -> value = std::rand();
        nextv.store(nullptr);
        ts = -1;
    }
    Vnode(V val){
        this -> value = val;
        nextv.store(nullptr);
        ts = -1;
    }
    Vnode(V val, Vnode<V>* n){
        this -> value = val;
        nextv.store(n);
        ts = -1;
    }
};

template<typename K, typename V>
class ll_Node{
public:
    K key;
    std::atomic<Vnode<V>*> vhead;
    std::atomic<ll_Node<K,V>*> next;
    std::atomic<int64_t> del;
    ll_Node(K key, V value)
    {
        this -> key = key;
        Vnode<V> *temp = new Vnode(value);
        vhead.store(temp, std::memory_order_seq_cst);
        next.store(nullptr, std::memory_order_seq_cst);
        del = 0;
    }

    ll_Node(K key, V value, ll_Node<K,V>* next)
    {
        this -> key = key;
        Vnode<V> *temp = new Vnode(value);
        vhead.store(temp, std::memory_order_seq_cst);
        this -> next.store(next, std::memory_order_seq_cst);
        del = 0;
    }
    ll_Node(K key, Vnode<V>* vhead, ll_Node<K,V>* next)
    {
        this -> key = key;
        this -> vhead.store(vhead, std::memory_order_seq_cst);
        this -> next.store(next, std::memory_order_seq_cst);
        del = 0;
    }
};

ll_Node<uint64_t, int64_t>* dummy_node = new ll_Node<uint64_t, int64_t>(-1, -1);

template<typename K, typename V>
class State{
public:
    int64_t phase;
    K key;
    V value;
    Vnode<V>* vnode;
    bool finished;
    std::atomic<ll_Node<K,V>*> search_node;
    State(int64_t __phase, K __key, V __value)
    {
        phase = __phase;
        key = __key;
        value = __value;
        vnode = new Vnode<V>(value);
        finished = false;
        search_node.store(dummy_node, std::memory_order_seq_cst);
    }
};

std::vector<State<uint64_t, int64_t>*> stateArray;

class HelpRecord
{
public:
    int curr_tid;
    long lastPhase;
    long nextCheck;
    HelpRecord(int tid)
    {
        curr_tid = -1;
        reset();
    }
    void reset()
    {
        curr_tid = (curr_tid + 1)% NUM_THREADS;
        if(stateArray[curr_tid] == nullptr)
            lastPhase = -1;
        else
            lastPhase = stateArray[curr_tid] -> phase;
        nextCheck = HELPING_DELAY;
    }
};
static thread_local HelpRecord* help_obj;


int64_t is_marked(uintptr_t i)
{
    return (int64_t)(i & (uintptr_t)0x01);
}

uintptr_t set_mark(uintptr_t i)
{
    return (i | (uintptr_t)0x01);
}

uintptr_t unset_mark(uintptr_t i)
{
    return (i & ~(uintptr_t)0x01);
}

int64_t is_marked_ref(long i)
{
    /* return (int64_t) (i & (LONG_MIN+1)); */
    return (int64_t) (i & 0x1L);
}

long unset_mark(long i)
{
    /* i &= LONG_MAX-1; */
    i &= ~0x1L;
    return i;
}

long set_mark(long i)
{
    /* i = unset_mark(i); */
    /* i += 1; */
    i |= 0x1L;
    return i;
}

long get_unmarked_ref(long w)
{
    /* return unset_mark(w); */
    return w & ~0x1L;
}

TrackerList version_tracker;

#endif //UNTITLED_UTIL_H
