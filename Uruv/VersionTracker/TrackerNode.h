#ifndef TIMESTAMP_TRACKER_TRACKERNODE_H
#define TIMESTAMP_TRACKER_TRACKERNODE_H

#include <atomic>

class TrackerNode{
public:
	int64_t ts;
	std::atomic<TrackerNode*> next;
	bool finish;
	
	TrackerNode(){
		ts = -1;
		next = nullptr;
		finish = false;
	}
	
	TrackerNode(int64_t init_ts){
		ts = init_ts;
		next = nullptr;
		finish = false;
	}
	
	TrackerNode(int64_t init_ts, TrackerNode* next_node){
		ts = init_ts;
		next = next_node;
		finish= false;
	}
};

#endif //TIMESTAMP_TRACKER_TRACKERNODE_H
