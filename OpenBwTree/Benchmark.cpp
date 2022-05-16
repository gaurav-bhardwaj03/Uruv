#include "bwtree.h"
#include <random>
#include <thread>
#include <fstream>
#include <cassert>
#include <unistd.h>
#include <algorithm>
#include <iostream>

int64_t table_size;
int64_t DatasetSize;
int64_t threads;
double Read;
double Insert;
double Delete;
double RangeQuery;
int64_t RangeQuerySize;
int64_t Runtime = 10;
bool running = false;
std::atomic<size_t> ready_threads (0);
std::vector<int64_t> tho;
std::vector<int64_t> dataset;
int dataset_prep_threads;
std::ofstream fout;

bool print_flag = true;
thread_local int wangziqi2013::bwtree::BwTreeBase::gc_id = -1;
std::atomic<size_t> wangziqi2013::bwtree::BwTreeBase::total_thread_num{500};
static const int64_t key_type=0;
wangziqi2013::bwtree::BwTree<int64_t, int64_t>* tree;

void prepare_table ();
void run_benchmark ();
void prepare_dataset ();
void read_from_files();

int main(int argc, char** argv)
{
	fout.open("final_benchmarks.txt", std::ios::app);
	if(argc != 10)
	{
		std::cout<<"Invalid Arguments \n";
		return 0;
	}
	threads = atoi(argv[1]);
	Read = ((double)atoi(argv[2]))/100;
	Insert = ((double)atoi(argv[3]))/100;
	Delete = ((double)atoi(argv[4]))/100;
	RangeQuery = ((double)atoi(argv[5]))/100;
	RangeQuerySize = atol(argv[6]);
	table_size = atol(argv[7]);
	DatasetSize = atol(argv[8]);
	dataset_prep_threads = atoi(argv[9]);
	if(threads == 0){
		std::cout<<"Error:: Threads cannot be Zero\n";
		return 0;
	}
	if(DatasetSize == 0)
	{
		std::cout<<"Error:: Dataset size cannot be zero\n";
		return 0;
	}
	printf("Parameters\nThreads: %ld\nRead: %lf\nInsert: %lf\nDelete: %lf\nRangeQuery: %lf\nRangeQuerySize: %ld\n"
		   "TableSize: %ld\nDatasetSize: %ld\n", threads, Read, Insert, Delete, RangeQuery, RangeQuerySize, table_size, DatasetSize);
	assert(((int64_t)(Read + Delete + Insert + RangeQuery)) == 1);
	tree = new wangziqi2013::bwtree::BwTree<int64_t , int64_t, std::less<int64_t>, std::equal_to<int64_t>, std::hash<int64_t>>();
	std::cout << "tree built\n";
	tho.resize(threads, 0);
	dataset = std::vector<int64_t> (DatasetSize);
	read_from_files();
	std::cout<<"Dataset Prepared\n";
	if(table_size > 0)
		prepare_table ();
	std::cout<<"Table Prepared\n";
	run_benchmark ();
	fout.close();
}

void run (int64_t tid) {
	tree -> AssignGCID(tid + 160);
	std::uniform_real_distribution<double> ratio_dis (0, 1);
	std::uniform_int_distribution<int64_t> index_dis(0, dataset.size() - 1);
	ready_threads++;
	std::random_device rd;
	std::mt19937 gen(rd());
	int64_t j = index_dis(gen);
	while (!running);
	while (running) {
		double d = (double) ratio_dis (gen);
		if (d <= Read) {
			tree -> GetValue((uint64_t)dataset[j]);   // read
		} else if (d <= Read + Delete) {
			tree -> Delete(dataset[j], j);  // delete
		} else if (d <= Read + Delete + Insert) {
			tree -> Insert(dataset[j], j);  // insert
		}
		j = (j + 1)%dataset.size();
		tho[tid]++;
	}
}

void tree_preparation(int tid, int64_t start, int64_t fin){
	tree ->AssignGCID(tid);
	for(int64_t i = start; i < fin ; i++) {
		tree->Insert(i, 10);
	}
}

void prepare_table () {
	std::vector<std::thread> tree_prep_threads(dataset_prep_threads);
	int64_t number_of_keys_per_thread = table_size/dataset_prep_threads;
	for(int64_t i = 0; i < dataset_prep_threads; i++){
		int64_t fin = std::min((i + 1) * number_of_keys_per_thread, table_size);
		tree_prep_threads[i] = std::thread( tree_preparation, i ,  i * (number_of_keys_per_thread), fin);
	}
	for(int64_t i = 0; i < dataset_prep_threads; i++){
		tree_prep_threads[i].join();
	}
}

void run_benchmark () {
	std::thread t[threads];
	//Thread pinning logic. Change this according to your machine.
	int64_t numa1 = 0, numa2 = 40, numa3 = 80, numa4 = 120;
	for (int64_t i = 0; i < threads; i++) {
		tho[i] = 0;
		t[i] = std::thread (run, i);
		cpu_set_t cpuset;
		int64_t cpu_pin;
		if(i < 80){
			if(i % 2 == 0){
				cpu_pin = numa1;
				numa1++;
			}
			else{
				cpu_pin = numa2;
				numa2++;
			}
		}
		else{
			if(i % 2 == 0){
				cpu_pin = numa3;
				numa3++;
			}
			else{
				cpu_pin = numa4;
				numa4++;
			}
		}
		CPU_ZERO(&cpuset);
		CPU_SET(cpu_pin, &cpuset);
		pthread_setaffinity_np (t[i].native_handle (),
								sizeof (cpu_set_t), &cpuset);
	}
	while (ready_threads < threads) sleep (1);
	running = true;
	sleep(Runtime);
	running = false;
	for (int64_t i = 0; i < threads; ++i)
		t[i].join ();
	uint64_t throughput = 0;
	for (int64_t i = 0; i < threads; i++)
		throughput += tho[i];
	std::cout<< threads << " " << Read <<    " "   << Insert << " "  << Delete << " "  << RangeQuery << " " << RangeQuerySize << " " << table_size << " " << DatasetSize << " "
			 << throughput / Runtime << "\n";
	fout<< threads << " " << Read <<    " "   << Insert << " "  << Delete << " "  << RangeQuery << " " << RangeQuerySize << " " << table_size << " " << DatasetSize << " " <<
		throughput / Runtime << "\n";
}

void file_reader(int64_t tid, int64_t start){
	std::ifstream reader;
	reader.open("../DatasetGen/Data/" + std::to_string(DatasetSize/1000000)+ "M/Data_Split_" + std::to_string(tid), std::ios::in);
	if(!reader.is_open())
	{
		std::cout<<"Error:: Dataset does not exist\n";
		exit(1);
	}
	std::string number;
	int64_t index = start;
	while(reader >> number){
		dataset[index] = std::stoll(number);
		index++;
	}
	reader.close();
}

void read_from_files(){
	std::vector<std::thread> reader_threads(dataset_prep_threads);
	int64_t number_of_keys_per_thread = DatasetSize/dataset_prep_threads;
	for(int64_t i = 0; i < dataset_prep_threads; i++){
		int64_t begin = i * number_of_keys_per_thread;
		int64_t fin = std::min((i + 1) * number_of_keys_per_thread, DatasetSize);
		reader_threads[i] = std::thread(file_reader, i, begin);
	}
	for(int64_t i = 0; i < dataset_prep_threads; i++){
		reader_threads[i].join();
	}
}